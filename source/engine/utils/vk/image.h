#pragma once

#include <engine/utils/vk/vk_driver.h>
#include <memory>
#include <vk_mem_alloc.h>

namespace mango {
class StagePool;
class CommandBuffer;
class Image final {
public:
  Image(const std::shared_ptr<VkDriver> &driver, VkImageCreateFlags flags,
        VkFormat format, const VkExtent3D &extent,
        VkSampleCountFlagBits sample_count, VkImageUsageFlags image_usage,
        VmaMemoryUsage memory_usage);

  Image(const Image &) = delete;
  Image(Image &&) = delete;
  Image &operator=(const Image &) = delete;

  ~Image();

  VkImage getHandle() const { return image_; }

  /**
   * update image from cpu to gpu, data should be compatiable with image format,
   * and tightly packed.
   */
  void updateByStaging(void *data, const std::shared_ptr<StagePool> &stage_pool,
                       const std::shared_ptr<CommandBuffer> &cmd_buf);

  std::shared_ptr<VkDriver> getDriver() const { return driver_; }

  void transitionLayout(VkCommandBuffer cmd_buf,
                        const VkImageSubresourceRange &range,
                        const VkImageLayout layout);

  // VkImageLayout getDefaultLayout() const;

private:
  std::shared_ptr<VkDriver> driver_;
  VkImageCreateFlags flags_;
  VkFormat format_;
  VkExtent3D extent_;
  VkSampleCountFlagBits sample_count_;
  VkImageUsageFlags image_usage_;
  VmaMemoryUsage memory_usage_;

  VmaAllocation allocation_{VK_NULL_HANDLE};
  VkImage image_{VK_NULL_HANDLE};
  VkImageLayout layout_{VK_IMAGE_LAYOUT_UNDEFINED};
};

class ImageView final {
public:
  ImageView(const std::shared_ptr<Image> &image, VkImageViewType view_type,
            VkFormat format, VkImageAspectFlags aspect_flags,
            uint32_t base_mip_level, uint32_t base_array_layer,
            uint32_t n_mip_levels, uint32_t n_array_layers);

  ImageView(const std::shared_ptr<VkDriver> &driver, VkImage image,
            VkImageViewType view_type, VkFormat format,
            VkImageAspectFlags aspect_flags, uint32_t base_mip_level,
            uint32_t base_array_layer, uint32_t n_mip_levels,
            uint32_t n_array_layers);

  ImageView(const ImageView &) = delete;
  ImageView(ImageView &&) = delete;
  ImageView &operator=(const ImageView &) = delete;

  VkImageView getHandle() const { return image_view_; }

  VkImage getVkImage() const { return vk_image_; }

#ifndef NDEBUG
  VkImageSubresourceRange getSubresourceRange() const {
    return subresource_range_;
  }
#endif

  ~ImageView();

private:
#ifndef NDEBUG
  VkImageViewType view_type_;
  VkFormat format_;
  VkImageSubresourceRange subresource_range_;
#endif
  VkImage vk_image_{VK_NULL_HANDLE};
  VkImageView image_view_{VK_NULL_HANDLE};

  std::shared_ptr<VkDriver> driver_;
  std::shared_ptr<Image> image_ptr_; //!< used to keep image alive
};
} // namespace mango