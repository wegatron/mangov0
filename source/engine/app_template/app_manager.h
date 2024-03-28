#pragma once

#include <memory>
#include <string>

#include <GLFW/glfw3.h>

#include <engine/app_template/app_base.h>
#include <engine/platform/window.h>
#include <engine/utils/vk/frame_buffer.h>
#include <engine/utils/vk/swapchain.h>
#include <engine/utils/vk/vk_driver.h>

namespace mango {
class Application final {
public:
  Application(std::unique_ptr<Window> &&window);

  ~Application();

  Application(const Application &) = delete;

  Application &operator=(const Application &) = delete;

  bool init(const std::shared_ptr<VkConfig> &config, VkFormat color_format,
            VkFormat ds_format);

private:
  // void resize(uint32_t width, uint32_t height);

  void initSwapchain();

  void initRenderTargets();

  std::shared_ptr<VkDriver> driver_;
  std::shared_ptr<Swapchain> swapchain_;
  std::vector<std::shared_ptr<RenderTarget>> render_targets_;

  VkFormat color_format_;
  VkFormat ds_format_;
  std::vector<std::shared_ptr<Image>> depth_images_;

  std::shared_ptr<AppBase> app_;
  std::unique_ptr<Window> window_;

  uint32_t width_;
  uint32_t height_;
  std::string window_title_;
  uint32_t current_frame_index_{0};
};
} // namespace mango