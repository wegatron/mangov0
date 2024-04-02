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

  void getMousePos(int32_t &xpos, int32_t &ypos) override {
    xpos = xpos_;
    ypos = ypos_;
  }

  VkSurfaceKHR createSurface(VkInstance instance) override;

  void *getHandle() override { return window_; }

private:
  void setupCallback();

  static void keyCallback(GLFWwindow *window, int key, int scancode, int action,
                          int mods);
  static void charCallback(GLFWwindow *window, unsigned int codepoint);
  static void charModsCallback(GLFWwindow *window, unsigned int codepoint,
                               int mods);
  static void mouseButtonCallback(GLFWwindow *window, int button, int action,
                                  int mods);
  static void cursorPosCallback(GLFWwindow *window, double xpos, double ypos);
  static void cursorEnterCallback(GLFWwindow *window, int entered);
  static void scrollCallback(GLFWwindow *window, double xoffset,
                             double yoffset);
  static void dropCallback(GLFWwindow *window, int count, const char **paths);
  static void windowSizeCallback(GLFWwindow *window, int width, int height);
  static void windowCloseCallback(GLFWwindow *window);

  GLFWwindow *window_;

  int32_t xpos_;
  int32_t ypos_;
};
} // namespace mango