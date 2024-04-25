#include <engine/utils/base/error.h>
#include <engine/utils/vk/commands.h>
#include <engine/utils/vk/image.h>
#include <engine/utils/vk/stage_pool.h>

namespace mango {

Image::Image(const std::shared_ptr<VkDriver> &driver, VkImageCreateFlags flags,
             VkFormat format, const VkExtent3D &extent, uint32_t mip_levels,
             uint32_t array_layers, VkSampleCountFlagBits sample_count,
             VkImageUsageFlags image_usage, VmaMemoryUsage memory_usage,
             VkImageLayout layout)
    : driver_(driver), flags_(flags), format_(format), extent_(extent),
      sample_count_(sample_count), image_usage_(image_usage),
      memory_usage_(memory_usage) {
  VkImageCreateInfo image_info = {};
  image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  image_info.flags = flags;
  image_info.imageType = VK_IMAGE_TYPE_2D;
  image_info.format = format_;
  image_info.extent = extent_;
  image_info.mipLevels = mip_levels;
  image_info.arrayLayers = array_layers;
  image_info.samples = sample_count_;
  image_info.tiling = VK_IMAGE_TILING_OPTIMAL;
  image_info.usage = image_usage_;
  image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  image_info.initialLayout = layout;

  VmaAllocationCreateInfo alloc_create_info = {};
  alloc_create_info.usage = memory_usage;

  // used for for memory less attachment, like depth stencil
  if (image_usage & VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT) {
    alloc_create_info.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
  }

  auto result =
      vmaCreateImage(driver_->getAllocator(), &image_info, &alloc_create_info,
                     &image_, &allocation_, nullptr);
  if (result != VK_SUCCESS) {
    throw VulkanException(result, "failed to create image!");
  }
}

Image::~Image() {
  vmaDestroyImage(driver_->getAllocator(), image_, allocation_);
}

void Image::updateByStaging(const void *data,
                            const std::shared_ptr<CommandBuffer> &cmd_buf) {
  uint32_t pixel_size = 0;
  if (format_ == VK_FORMAT_R8G8B8_SRGB) {
    pixel_size = 3;
  } else if (format_ == VK_FORMAT_R8G8B8A8_SRGB) {
    pixel_size = 4;
  } else if (format_ == VK_FORMAT_R8_UNORM) {
    pixel_size = 1;
  } else if (format_ == VK_FORMAT_R32G32B32A32_SFLOAT) {
    pixel_size = 16;
  }

  if (pixel_size == 0) {
    throw std::runtime_error("Unsupported image format for update by staging.");
  }
  auto data_size = extent_.width * extent_.height * pixel_size;
  auto stage_pool = driver_->getStagePool();
  auto stage = stage_pool->acquireStage(data_size);

  // cpu data to staging
  void *mapped;
  vmaMapMemory(driver_->getAllocator(), stage->memory, &mapped);
  memcpy(mapped, data, data_size);
  vmaUnmapMemory(driver_->getAllocator(), stage->memory);
  vmaFlushAllocation(driver_->getAllocator(), stage->memory, 0, data_size);

  // staging buffer to image
  VkBufferImageCopy copyRegion = {
      .bufferOffset = {},
      .bufferRowLength = {},
      .bufferImageHeight = {},
      .imageSubresource = {.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                           .mipLevel = 0,
                           .baseArrayLayer = 0,
                           .layerCount = 1},
      .imageOffset = {0, 0, 0},
      .imageExtent = extent_};

  VkImageSubresourceRange transitionRange = {.aspectMask =
                                                 VK_IMAGE_ASPECT_COLOR_BIT,
                                             .baseMipLevel = 0,
                                             .levelCount = 1,
                                             .baseArrayLayer = 0,
                                             .layerCount = 1};

  auto cmd_buf_handle = cmd_buf->getHandle();
  transitionLayout(cmd_buf_handle, transitionRange,
                   VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

  vkCmdCopyBufferToImage(cmd_buf_handle, stage->buffer, image_,
                         VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyRegion);

  // transitionLayout(cmd_buf_handle, transitionRange, getDefaultLayout());
}

void setTransSrcDst(const VkImageLayout old_layout,
                    const VkImageLayout new_layout,
                    VkAccessFlags &src_access_mask,
                    VkAccessFlags &dst_access_mask,
                    VkPipelineStageFlags &src_stage,
                    VkPipelineStageFlags &dst_stage) {
  switch (new_layout) {
  case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
    src_access_mask = 0;
    dst_access_mask = VK_ACCESS_TRANSFER_WRITE_BIT;
    src_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    dst_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    break;
  case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
    src_access_mask = 0;
    dst_access_mask = VK_ACCESS_TRANSFER_READ_BIT;
    src_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    dst_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    break;
  case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
  case VK_IMAGE_LAYOUT_GENERAL:
  case VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL:
    src_access_mask = VK_ACCESS_TRANSFER_WRITE_BIT;
    dst_access_mask = VK_ACCESS_SHADER_READ_BIT;
    src_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    dst_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    break;

    // We support PRESENT as a target layout to allow blitting from the swap
    // chain. See also SwapChain::makePresentable().
  case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
  case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
    src_access_mask = VK_ACCESS_TRANSFER_READ_BIT;
    dst_access_mask = 0;
    src_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    dst_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    break;

  default:
    throw std::runtime_error("Unsupported layout transition.");
  }
}

void Image::transitionLayout(const VkCommandBuffer cmd_buf,
                             const VkImageSubresourceRange &range,
                             const VkImageLayout layout) {
  if (layout_ == layout)
    return;
  // src dst
  VkAccessFlags src_access_mask, dst_access_mask;
  VkPipelineStageFlags src_stage, dst_stage;
  setTransSrcDst(layout_, layout, src_access_mask, dst_access_mask, src_stage,
                 dst_stage);
  VkImageMemoryBarrier barrier = {};
  barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
  barrier.oldLayout = layout_;
  barrier.newLayout = layout;
  barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.image = image_;
  barrier.subresourceRange = range;
  barrier.srcAccessMask = src_access_mask;
  barrier.dstAccessMask = dst_access_mask;
  vkCmdPipelineBarrier(cmd_buf, src_stage, dst_stage, 0, 0, nullptr, 0, nullptr,
                       1, &barrier);
  layout_ = layout;
}

// VkImageLayout Image::getDefaultLayout() const
// {
//     // Filament sometimes samples from depth while it is bound to the current
//     render target, (e.g.
//     // SSAO does this while depth writes are disabled) so let's keep it
//     simple and use GENERAL for
//     // all depth textures.
//     bool sample_flag = image_usage_ & VK_IMAGE_USAGE_SAMPLED_BIT;

//     bool attachment_flag =
//         (image_usage_ & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) ||
//         (image_usage_ & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
//     if ((sample_flag && attachment_flag) || image_usage_ &
//     VK_IMAGE_USAGE_STORAGE_BIT)
//       return VK_IMAGE_LAYOUT_GENERAL;

//     if(sample_flag)
//       return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

//     return VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL;
// }

ImageView::ImageView(const std::shared_ptr<Image> &image,
                     VkImageViewType view_type, VkFormat format,
                     VkImageAspectFlags aspect_flags, uint32_t base_mip_level,
                     uint32_t base_array_layer, uint32_t n_mip_levels,
                     uint32_t n_array_layers)
    : driver_(image->getDriver()), vk_image_(image->getHandle()),
      image_ptr_(image) {
  VkImageViewCreateInfo view_info = {};
  view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  view_info.image = image->getHandle();
  view_info.viewType = view_type;
  view_info.format = format;
  view_info.subresourceRange.aspectMask = aspect_flags;
  view_info.subresourceRange.baseMipLevel = base_mip_level;
  view_info.subresourceRange.levelCount = n_mip_levels;
  view_info.subresourceRange.baseArrayLayer = base_array_layer;
  view_info.subresourceRange.layerCount = n_array_layers;

  view_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
  view_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
  view_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
  view_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

#ifndef NDEBUG
  view_type_ = view_type;
  format_ = format;
  subresource_range_ = view_info.subresourceRange;
#endif

  auto result = vkCreateImageView(image->getDriver()->getDevice(), &view_info,
                                  nullptr, &image_view_);
  if (result != VK_SUCCESS) {
    throw VulkanException(result, "failed to create image view!");
  }
}

ImageView::ImageView(const std::shared_ptr<VkDriver> &driver, VkImage image,
                     VkImageViewType view_type, VkFormat format,
                     VkImageAspectFlags aspect_flags, uint32_t base_mip_level,
                     uint32_t base_array_layer, uint32_t n_mip_levels,
                     uint32_t n_array_layers)
    : driver_(driver), vk_image_(image) {
  VkImageViewCreateInfo view_info = {};
  view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  view_info.image = image;
  view_info.viewType = view_type;
  view_info.format = format;
  view_info.subresourceRange.aspectMask = aspect_flags;
  view_info.subresourceRange.baseMipLevel = base_mip_level;
  view_info.subresourceRange.levelCount = n_mip_levels;
  view_info.subresourceRange.baseArrayLayer = base_array_layer;
  view_info.subresourceRange.layerCount = n_array_layers;

  view_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
  view_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
  view_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
  view_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

#ifndef NDEBUG
  view_type_ = view_type;
  format_ = format;
  subresource_range_ = view_info.subresourceRange;
#endif

  auto result =
      vkCreateImageView(driver->getDevice(), &view_info, nullptr, &image_view_);
  if (result != VK_SUCCESS) {
    throw VulkanException(result, "failed to create image view!");
  }
}

ImageView::~ImageView() {
  vkDestroyImageView(driver_->getDevice(), image_view_, nullptr);
}

} // namespace mango