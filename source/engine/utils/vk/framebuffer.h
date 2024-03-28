#pragma once

#include <engine/utils/vk/image.h>
#include <engine/utils/vk/render_pass.h>
#include <engine/utils/vk/vk_driver.h>
#include <volk.h>

namespace mango {

/**
 * \brief render target is a collection of images used for rendering.
 * use this class for render target's creation and management.
 */
class RenderTarget final {
public:
  ~RenderTarget();

  RenderTarget(const RenderTarget &) = delete;
  RenderTarget &operator=(const RenderTarget &) = delete;

  RenderTarget(const std::shared_ptr<VkDriver> &driver,
               const std::initializer_list<VkFormat> &color_format,
               VkFormat ds_format, uint32_t width, uint32_t height,
               uint32_t layers);

  RenderTarget(
      const std::initializer_list<std::shared_ptr<ImageView>> &image_views,
      const std::initializer_list<VkFormat> &color_format, VkFormat ds_format,
      uint32_t width, uint32_t height, uint32_t layers);

  const std::vector<std::shared_ptr<ImageView>> &getImageViews() const {
    return images_views_;
  }

  uint32_t getWidth() const { return width_; }

  uint32_t getHeight() const { return height_; }

  uint32_t getLayers() const { return layers_; }

  VkFormat getColorFormat(uint32_t index) const {
    return color_formats_[index];
  }

  VkFormat getDSFormat() const { return ds_format_; }

private:
  uint32_t width_{0};
  uint32_t height_{0};
  uint32_t layers_{1}; // should be one, for multiview

  std::shared_ptr<VkDriver> driver_;
  std::vector<std::shared_ptr<ImageView>> images_views_;
  std::vector<std::shared_ptr<Image>> images_;
  std::vector<VkFormat> color_formats_;
  VkFormat ds_format_{VK_FORMAT_UNDEFINED}; // undefined means no depth-stencil
};

/**
 * \brief framebuffer is a combination of render target and render pass,
 * and used for manage the VkFramebuffer.
 *
 * render pass defining what render passes the framebuffer will be compatible
 * with.
 */
class FrameBuffer final {
public:
  FrameBuffer(const std::shared_ptr<VkDriver> &driver,
              const std::shared_ptr<RenderPass> &render_pass,
              const std::shared_ptr<RenderTarget> &render_target);

  FrameBuffer(const std::shared_ptr<VkDriver> &driver,
              const std::shared_ptr<RenderPass> &render_pass,
              const std::shared_ptr<RenderTarget> &render_target,
              const uint32_t sub_image_views_count);

  FrameBuffer(const FrameBuffer &) = delete;
  FrameBuffer &operator=(const FrameBuffer &) = delete;
  FrameBuffer(FrameBuffer &&) = delete;

  ~FrameBuffer();

  VkFramebuffer getHandle() const { return framebuffer_; }

  uint32_t getWidth() const { return render_target_->getWidth(); }

  uint32_t getHeight() const { return render_target_->getHeight(); }

  uint32_t getLayers() const { return render_target_->getLayers(); }

  std::shared_ptr<RenderTarget> getRenderTarget() const {
    return render_target_;
  }

private:
  std::shared_ptr<VkDriver> driver_;
  std::shared_ptr<RenderPass> render_pass_;
  std::shared_ptr<RenderTarget> render_target_;

  VkFramebuffer framebuffer_{VK_NULL_HANDLE};
};
} // namespace mango