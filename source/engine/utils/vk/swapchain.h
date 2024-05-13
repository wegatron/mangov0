#pragma once

#include <vector>
#include <volk.h>

#include <engine/utils/vk/image.h>
#include <engine/utils/vk/vk_driver.h>

namespace mango {

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

  ~Swapchain();

  Swapchain(const Swapchain &) = delete;
  Swapchain &operator=(const Swapchain &) = delete;

  VkSwapchainKHR getHandle() const { return swapchain_; }

  VkExtent2D getExtent() const { return extent_; }

  VkFormat getImageFormat() const { return image_format_; }

  std::shared_ptr<ImageView> getImageView(uint32_t index) const {
    assert(index < MAX_FRAMES_IN_FLIGHT);
    return image_views_[index];
  }

  VkResult acquireNextImage(VkSemaphore semaphore, VkFence fence, uint32_t &image_index);

  void initSwapchain(VkSurfaceKHR surface,
                     const SwapchainProperties &properties);

private:
  void initImages();

  VkSwapchainKHR swapchain_{VK_NULL_HANDLE};
  VkExtent2D extent_;
  VkFormat image_format_;
  VkImage images_[MAX_FRAMES_IN_FLIGHT];
  std::shared_ptr<ImageView> image_views_[MAX_FRAMES_IN_FLIGHT];
};

} // namespace mango