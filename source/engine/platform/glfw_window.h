#pragma once
#include <GLFW/glfw3.h>
#include <engine/platform/window.h>
#include <engine/utils/vk/vk_config.h>
#include <memory>

namespace mango {

class Swapchain;
class Image;
class GlfwWindow final : public Window {
public:
  GlfwWindow(const std::string &window_title, const int width,
             const int height);

  ~GlfwWindow() override;

  bool shouldClose() override;

  // void initImgui() override;

  // void shutdownImgui() override;

  // void imguiNewFrame() override;

  void processEvents() override { glfwPollEvents(); }

  void getWindowSize(uint32_t &width, uint32_t &height) override;

  void getMousePos(int32_t &xpos, int32_t &ypos) override;

  VkSurfaceKHR createSurface(VkInstance instance) override;

  void *getHandle() override { return window_; }

private:
  void setupCallback();

  GLFWwindow *window_;
};
} // namespace mango