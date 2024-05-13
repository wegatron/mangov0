#include <engine/functional/global/engine_context.h>
#include <engine/utils/base/error.h>
#include <engine/utils/vk/swapchain.h>

namespace mango {
Swapchain::Swapchain(VkSurfaceKHR surface,
                     const SwapchainProperties &properties) {

  initSwapchain(surface, properties);
}

void Swapchain::initSwapchain(VkSurfaceKHR surface,
                              const SwapchainProperties &properties) {
  auto driver = g_engine.getDriver();
  // check surface capabilities
  VkSurfaceCapabilitiesKHR surface_capabilities;
  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(driver->getPhysicalDevice(),
                                            surface, &surface_capabilities);
  extent_.width = std::clamp(properties.extent.width,
                             surface_capabilities.minImageExtent.width,
                             surface_capabilities.maxImageExtent.width);
  extent_.height = std::clamp(properties.extent.height,
                              surface_capabilities.minImageExtent.height,
                              surface_capabilities.maxImageExtent.height);
  assert(std::clamp(properties.image_count, surface_capabilities.minImageCount,
    surface_capabilities.maxImageCount) == MAX_FRAMES_IN_FLIGHT);
  image_format_ = properties.surface_format.format;

  // create swapchain
  VkSwapchainCreateInfoKHR swapchain_info{};
  swapchain_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  swapchain_info.surface = surface;
  swapchain_info.imageExtent = extent_;
  swapchain_info.minImageCount = MAX_FRAMES_IN_FLIGHT;

  auto old_swapchain = swapchain_;
  // imageFormat specifies what the image format is (same as it does in
  // vkCreateImage()). imageColorSpace is how the swapchain\display interprets
  // the values when the image is presented.
  swapchain_info.imageFormat = properties.surface_format.format;
  swapchain_info.imageColorSpace = properties.surface_format.colorSpace;
  swapchain_info.imageArrayLayers = 1;
  swapchain_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
  swapchain_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
  swapchain_info.preTransform = surface_capabilities.currentTransform;
  swapchain_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
  swapchain_info.clipped = VK_TRUE;
  swapchain_info.presentMode = VK_PRESENT_MODE_FIFO_KHR;
  swapchain_info.oldSwapchain = old_swapchain;

  // vkCreateSwapchainKHR maybe null if not enable VK_KHR_swapchain
  // make sure eanbleExtensionCount is correct when create logical device
  // refer to:
  // https://stackoverflow.com/questions/55131406/why-would-vkcreateswapchainkhr-result-in-an-access-violation-at-0
  VK_THROW_IF_ERROR(vkCreateSwapchainKHR(driver->getDevice(), &swapchain_info,
                                         nullptr, &swapchain_),
                    "vulkan failed to create swapchain");

  if (old_swapchain != VK_NULL_HANDLE) {
    for (auto i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) { image_views_[i].reset(); }      
    vkDestroySwapchainKHR(driver->getDevice(), old_swapchain, nullptr);
  }

  initImages();
}

void Swapchain::initImages() {
  auto driver = g_engine.getDriver();
  // get swapchain images
  uint32_t image_count;
  vkGetSwapchainImagesKHR(driver->getDevice(), swapchain_, &image_count,
                          nullptr);
  assert(image_count == MAX_FRAMES_IN_FLIGHT);
  vkGetSwapchainImagesKHR(driver->getDevice(), swapchain_, &image_count,
                          images_);
  std::vector<std::shared_ptr<Image>> wraped_images(image_count);

  for (auto i = 0; i < image_count; ++i) {
    wraped_images[i] = std::make_shared<Image>(
        driver, images_[i], false, image_format_,
        VkExtent3D{extent_.width, extent_.height, 1}, 1, 1,
        VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
  }
  // image views
  for (auto i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
    image_views_[i] = std::make_shared<ImageView>(
        wraped_images[i], VK_IMAGE_VIEW_TYPE_2D, image_format_,
        VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1, 1);
  }
}

Swapchain::~Swapchain() {
#if !NDEBUG
  for (auto &image_view : image_views_) {
    assert(image_view.use_count() == 1);
  }
#endif
  auto driver = g_engine.getDriver();  
  for (auto i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) { image_views_[i].reset(); }
  vkDestroySwapchainKHR(driver->getDevice(), swapchain_, nullptr);
}

VkResult Swapchain::acquireNextImage(VkSemaphore semaphore, VkFence fence, uint32_t &image_index) {
  auto driver = g_engine.getDriver();
  auto result = vkAcquireNextImageKHR(driver->getDevice(), swapchain_, UINT64_MAX, semaphore,
                        fence, &image_index);
  return result;
}

} // namespace mango