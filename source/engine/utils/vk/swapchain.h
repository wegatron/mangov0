#pragma once

#include <vector>
#include <volk.h>

#include <engine/utils/vk/image.h>
#include <engine/utils/vk/vk_driver.h>

namespace mango {
class RenderPass;
class FrameBuffer;

enum ImageFormat { sRGB, UNORM };

struct SwapchainProperties {
  VkExtent2D extent{};
  VkSurfaceFormatKHR surface_format{VK_FORMAT_B8G8R8A8_SRGB,
                                    VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};
  VkSwapchainKHR old_swapchain{VK_NULL_HANDLE};
  uint32_t image_count{3};
  uint32_t array_layers{1};
  VkCompositeAlphaFlagBitsKHR composite_alpha{
      VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR};
  VkPresentModeKHR present_mode{VK_PRESENT_MODE_FIFO_KHR};
};

class Swapchain final {
public:
  Swapchain(VkSurfaceKHR surface, const SwapchainProperties &properties);

  void update(VkSurfaceKHR surface, const uint32_t width, const uint32_t height);

  ~Swapchain();

  Swapchain(const Swapchain &) = delete;
  Swapchain &operator=(const Swapchain &) = delete;

  VkSwapchainKHR getHandle() const { return swapchain_; }

  VkExtent2D getExtent() const { return extent_; }

  VkFormat getImageFormat() const { return image_format_; }

  void createFrameBuffer(const std::shared_ptr<RenderPass> &render_pass);

  std::shared_ptr<FrameBuffer> getFrameBuffer(const uint32_t index) const {
    assert(index < MAX_FRAMES_IN_FLIGHT);
    assert(framebuffers_[index] != nullptr);
    return framebuffers_[index];
  }

  VkResult acquireNextImage(VkSemaphore semaphore, VkFence fence, uint32_t &image_index);
  
private:
  void initSwapchain(VkSurfaceKHR surface,
                     const SwapchainProperties &properties);

  void initImages();

  VkSwapchainKHR swapchain_{VK_NULL_HANDLE};
  VkExtent2D extent_;
  VkFormat image_format_;
  VkImage images_[MAX_FRAMES_IN_FLIGHT];
  std::shared_ptr<ImageView> image_views_[MAX_FRAMES_IN_FLIGHT];
  std::shared_ptr<FrameBuffer> framebuffers_[MAX_FRAMES_IN_FLIGHT];
};

} // namespace mango