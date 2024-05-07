#include <engine/utils/base/compiler.h>
#include <engine/utils/base/error.h>
#include <engine/utils/vk/stage_pool.h>
#include <engine/utils/vk/vk_common.h>
#include <engine/utils/vk/vk_constants.h>

namespace mango {
// Finds or creates a stage whose capacity is at least the given number of
// bytes. The stage is automatically released back to the pool after
// TIME_BEFORE_EVICTION frames.
VulkanStage const *StagePool::acquireStage(uint32_t numBytes) {
  // First check if a stage exists whose capacity is greater than or equal to
  // the requested size.
  auto iter = free_stages_.lower_bound(numBytes);
  if (iter != free_stages_.end()) {
    auto stage = iter->second;
    free_stages_.erase(iter);
    used_stages_.insert(stage);
    return stage;
  }
  // We were not able to find a sufficiently large stage, so create a new one.
  VulkanStage *stage = new VulkanStage({
      .memory = VK_NULL_HANDLE,
      .buffer = VK_NULL_HANDLE,
      .capacity = numBytes,
      .lastAccessed = current_frame_,
  });

  // Create the VkBuffer.
  used_stages_.insert(stage);
  VkBufferCreateInfo bufferInfo{
      .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
      .size = numBytes,
      .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
  };
  VmaAllocationCreateInfo allocInfo{
    .flags = VMA_ALLOCATION_CREATE_MAPPED_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
    .usage = VMA_MEMORY_USAGE_AUTO_PREFER_HOST,
  };
  UTILS_UNUSED_IN_RELEASE VkResult result =
      vmaCreateBuffer(driver_->getAllocator(), &bufferInfo, &allocInfo,
                      &stage->buffer, &stage->memory, nullptr);

  VK_THROW_IF_ERROR(result, "Create Staging buffer failed!");

  return stage;
}

// Images have VK_IMAGE_LAYOUT_GENERAL and must not be transitioned to any other
// layout
VulkanStageImage const *StagePool::acquireImage(VkFormat format, uint32_t width,
                                                uint32_t height,
                                                VkCommandBuffer cmd_buf) {
  for (auto image : free_images_) {
    if (image->format == format && image->width == width &&
        image->height == height) {
      free_images_.erase(image);
      used_images_.insert(image);
      return image;
    }
  }

  VulkanStageImage *image = new VulkanStageImage({
      .format = format,
      .width = width,
      .height = height,
      .lastAccessed = current_frame_,
  });

  used_images_.insert(image);

  const VkImageCreateInfo imageInfo = {
      .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
      .imageType = VK_IMAGE_TYPE_2D,
      .format = format,
      .extent = {width, height, 1},
      .mipLevels = 1,
      .arrayLayers = 1,
      .samples = VK_SAMPLE_COUNT_1_BIT,
      .tiling = VK_IMAGE_TILING_LINEAR,
      .usage =
          VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT};

  const VmaAllocationCreateInfo allocInfo{.flags =
                                              VMA_ALLOCATION_CREATE_MAPPED_BIT,
                                          .usage = VMA_MEMORY_USAGE_CPU_TO_GPU};

  VkResult result =
      vmaCreateImage(driver_->getAllocator(), &imageInfo, &allocInfo,
                     &image->image, &image->memory, nullptr);

  VK_THROW_IF_ERROR(result, "Create staging image failed!");

  VkImageAspectFlags aspectFlags = isDepthFormat(format)
                                       ? VK_IMAGE_ASPECT_DEPTH_BIT
                                       : VK_IMAGE_ASPECT_COLOR_BIT;

  // We use VK_IMAGE_LAYOUT_GENERAL here because the spec says:
  // "Host access to image memory is only well-defined for linear images and for
  // image subresources of those images which are currently in either the
  // VK_IMAGE_LAYOUT_PREINITIALIZED or VK_IMAGE_LAYOUT_GENERAL layout. Calling
  // vkGetImageSubresourceLayout for a linear image returns a subresource layout
  // mapping that is valid for either of those image layouts."
  transitionImageLayout(
      cmd_buf,
      blitterTransitionHelper({.image = image->image,
                               .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                               .newLayout = VK_IMAGE_LAYOUT_GENERAL,
                               .subresources = {aspectFlags, 0, 1, 0, 1}}));

  return image;
}

// Evicts old unused stages and bumps the current frame number.
void StagePool::gc() noexcept {
  // If this is one of the first few frames, return early to avoid wrapping
  // unsigned integers.
  if (++current_frame_ <= TIME_BEFORE_EVICTION) {
    return;
  }
  const uint64_t evictionTime = current_frame_ - TIME_BEFORE_EVICTION;

  // Destroy buffers that have not been used for several frames.
  decltype(free_stages_) freeStages;
  freeStages.swap(free_stages_);
  for (auto pair : freeStages) {
    if (pair.second->lastAccessed < evictionTime) {
      vmaDestroyBuffer(driver_->getAllocator(), pair.second->buffer,
                       pair.second->memory);
      delete pair.second;
    } else {
      free_stages_.insert(pair);
    }
  }

  // Reclaim buffers that are no longer being used by any command buffer.
  decltype(used_stages_) usedStages;
  usedStages.swap(used_stages_);
  for (auto stage : usedStages) {
    if (stage->lastAccessed < evictionTime) {
      stage->lastAccessed = current_frame_;
      free_stages_.insert(std::make_pair(stage->capacity, stage));
    } else {
      used_stages_.insert(stage);
    }
  }

  // Destroy images that have not been used for several frames.
  decltype(free_images_) freeImages;
  freeImages.swap(free_images_);
  for (auto image : freeImages) {
    if (image->lastAccessed < evictionTime) {
      vmaDestroyImage(driver_->getAllocator(), image->image, image->memory);
      delete image;
    } else {
      free_images_.insert(image);
    }
  }

  // Reclaim images that are no longer being used by any command buffer.
  decltype(used_images_) usedImages;
  usedImages.swap(used_images_);
  for (auto image : usedImages) {
    if (image->lastAccessed < evictionTime) {
      image->lastAccessed = current_frame_;
      free_images_.insert(image);
    } else {
      used_images_.insert(image);
    }
  }
}

// Destroys all unused stages and asserts that there are no stages currently in
// use. This should be called while the context's VkDevice is still alive.
void StagePool::reset() noexcept {
  for (auto stage : used_stages_) {
    vmaDestroyBuffer(driver_->getAllocator(), stage->buffer, stage->memory);
    delete stage;
  }
  used_stages_.clear();

  for (auto pair : free_stages_) {
    vmaDestroyBuffer(driver_->getAllocator(), pair.second->buffer,
                     pair.second->memory);
    delete pair.second;
  }
  free_stages_.clear();

  for (auto image : used_images_) {
    vmaDestroyImage(driver_->getAllocator(), image->image, image->memory);
    delete image;
  }
  used_images_.clear();

  for (auto image : free_images_) {
    vmaDestroyImage(driver_->getAllocator(), image->image, image->memory);
    delete image;
  }
  free_images_.clear();
}
} // namespace mango