#include <cassert>
#include <engine/app_template/app_manager.h>
#include <engine/functional/global/app_context.h>
#include <engine/platform/glfw_window.h>
#include <engine/utils/base/macro.h>
#include <engine/utils/vk/image.h>
#include <engine/utils/vk/queue.h>
#include <engine/utils/vk/syncs.h>
#include <engine/utils/vk/vk_driver.h>

namespace mango {

AppManager::AppManager(std::unique_ptr<Window> &&window) {
  assert(window != nullptr);
  window_ = std::move(window);
  window_->getExtent(width_, height_);
}

AppManager::~AppManager() {

  driver_->waitIdle();

  app_.reset();
  // destroy resources in global context
  destroyDefaultAppContext();
  // destroy render targets(ImageView) before swapchain and depth images
  render_targets_.clear();
  depth_images_.clear();
  swapchain_.reset();
  driver_.reset();
  window_.reset();
  // destroy swapchain and depth images implicitly
}

bool AppManager::init(const std::shared_ptr<VkConfig> &config,
                      VkFormat color_format, VkFormat ds_format) {
  color_format_ = color_format;
  ds_format_ = ds_format;
  driver_ = std::make_shared<VkDriver>();
  try {
#ifdef NDEBUG
    driver_->init(window_title_, config, window_);
#else
    config->setFeatureEnabled(VkConfig::FeatureExtension::KHR_VALIDATION_LAYER,
                              VkConfig::EnableState::REQUIRED);
    driver_->init(window_title_, config, window_.get());
#endif
  } catch (std::runtime_error &e) {
    const char *str = e.what();
    spdlog::error(str);
    // LOGE(str);
    return false;
  }

  // create swapchain
  initSwapchain();
  initRenderTargets();

  assert(app_ != nullptr);
  window_->setupCallback(app_.get());
  app_->init(window_.get(), driver_, render_targets_);
  return true;
}

void AppManager::run() {
  while (!window_->shouldClose()) {
    window_->processEvents();
    uint32_t width, height;
    window_->getExtent(width, height);
    resize(width, height);

    auto &render_output_sync =
        getDefaultAppContext().render_output_syncs[current_frame_index_];
    uint32_t rt_index = swapchain_->acquireNextImage(
        render_output_sync.present_semaphore->getHandle(), VK_NULL_HANDLE);
    // current_frame_index_ maybe different with rt_index, depends on the
    // swapchain present mode
    app_->tick(0.016f, rt_index, current_frame_index_); // 60fps

    // present
    VkPresentInfoKHR present_info{VK_STRUCTURE_TYPE_PRESENT_INFO_KHR};
    VkSwapchainKHR swapchain = swapchain_->getHandle();
    VkSemaphore render_semaphore =
        render_output_sync.render_semaphore->getHandle();
    present_info.swapchainCount = 1;
    present_info.pSwapchains = &swapchain;
    present_info.pImageIndices = &rt_index;
    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores = &render_semaphore;

    // Present swapchain image
    driver_->getGraphicsQueue()->present(present_info);
    current_frame_index_ = (current_frame_index_ + 1) % render_targets_.size();
  }
}

void AppManager::resize(uint32_t width, uint32_t height) {
  if (width_ == width && height_ == height)
    return;

  vkDeviceWaitIdle(driver_->getDevice());
  width_ = width;
  height_ = height;
  initSwapchain();
  initRenderTargets();

  app_->updateRts(render_targets_);
}

void AppManager::initSwapchain() {
  SwapchainProperties properties{
      .extent = {width_, height_},
      .surface_format = {.format = VK_FORMAT_B8G8R8A8_SRGB},
  };
  if (swapchain_ == nullptr)
    swapchain_ =
        std::make_shared<Swapchain>(driver_, driver_->getSurface(), properties);
  else {
    swapchain_->initSwapchain(properties);
  }
}

void AppManager::initRenderTargets() {
  // depth stencil images
  const auto img_cnt = swapchain_->getImageCount();
  VkExtent3D extent{width_, height_, 1};
  render_targets_.resize(img_cnt);
  depth_images_.resize(img_cnt);

  for (uint32_t i = 0; i < img_cnt; ++i) {
    depth_images_[i] = std::make_shared<Image>(
        driver_, 0, ds_format_, extent, VK_SAMPLE_COUNT_1_BIT,
        VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
        VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE);
    auto depth_img_view = std::make_shared<ImageView>(
        depth_images_[i], VK_IMAGE_VIEW_TYPE_2D, ds_format_,
        VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT, 0, 0, 1, 1);
    render_targets_[i] = std::make_shared<RenderTarget>(
        std::initializer_list<std::shared_ptr<ImageView>>{
            swapchain_->getImageView(i), depth_img_view},
        std::initializer_list<VkFormat>{swapchain_->getImageFormat()},
        ds_format_, extent.width, extent.height, 1u);
  }
}
} // namespace mango
