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
        VkFormat format, const VkExtent3D &extent, uint32_t mip_levels,
        uint32_t array_layers, VkSampleCountFlagBits sample_count,
        VkImageUsageFlags image_usage, VmaMemoryUsage memory_usage,
        VkImageLayout layout = VK_IMAGE_LAYOUT_UNDEFINED);

  Image(const std::shared_ptr<VkDriver> &driver, VkImage vk_image,
        bool own_image, VkFormat format, const VkExtent3D &extent,
        uint32_t mip_levels, uint32_t array_layers,
        VkSampleCountFlagBits sample_count, VkImageUsageFlags image_usage,
        VkImageLayout layout = VK_IMAGE_LAYOUT_UNDEFINED);

  Image(const Image &) = delete;
  Image(Image &&) = delete;
  Image &operator=(const Image &) = delete;

  ~Image();

  VkImage getHandle() const { return image_; }

  /**
   * update image from cpu to gpu, data should be compatiable with image format,
   * and tightly packed.
   */
  void updateByStaging(const void *data,
                       const std::shared_ptr<CommandBuffer> &cmd_buf);

  std::shared_ptr<VkDriver> getDriver() const { return driver_; }

  void transitionLayout(VkCommandBuffer cmd_buf,
                        const VkImageSubresourceRange &range,
                        const VkImageLayout layout);

private:
  void updateLayout(VkImageLayout layout) { layout_ = layout; }

  bool own_image_{true};
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

  friend class ImageView;
  friend class CommandBuffer;
};

class ImageView final {
public:
  ImageView(const std::shared_ptr<Image> &image, VkImageViewType view_type,
            VkFormat format, VkImageAspectFlags aspect_flags,
            uint32_t base_mip_level, uint32_t base_array_layer,
            uint32_t n_mip_levels, uint32_t n_array_layers);

  ImageView(const ImageView &) = delete;
  ImageView(ImageView &&) = delete;
  ImageView &operator=(const ImageView &) = delete;

  VkImageView getHandle() const { return image_view_; }

  std::shared_ptr<Image> getImage() const { return image_ptr_; }

  // VkImage getVkImage() const { return image_ptr_->getHandle(); }

#ifndef NDEBUG
  VkImageSubresourceRange getSubresourceRange() const {
    return subresource_range_;
  }
#endif

  ~ImageView();

  void transitionLayout(VkCommandBuffer cmd_buf, const VkImageLayout layout) {
    image_ptr_->transitionLayout(cmd_buf, subresource_range_, layout);
  }

private:
  void updateLayout(VkImageLayout layout) { image_ptr_->updateLayout(layout); }
#ifndef NDEBUG
  VkImageViewType view_type_;
  VkFormat format_;
  VkImageSubresourceRange subresource_range_;
#endif
  VkImageView image_view_{VK_NULL_HANDLE};

  std::shared_ptr<VkDriver> driver_;
  std::shared_ptr<Image> image_ptr_; //!< used to keep image alive
  friend class CommandBuffer;
};
} // namespace mango