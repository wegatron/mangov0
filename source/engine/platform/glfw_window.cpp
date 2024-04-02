#include <engine/functional/global/engine_context.h>
#include <engine/platform/glfw_window.h>
#include <engine/utils/base/error.h>
#include <engine/utils/event/event_system.h>
#include <engine/utils/vk/framebuffer.h>
#include <engine/utils/vk/image.h>
#include <engine/utils/vk/swapchain.h>

namespace mango {

GlfwWindow::GlfwWindow(const std::string &window_title, const int width,
                       const int height)
    : Window(window_title) {
#if defined(VK_USE_PLATFORM_XLIB_KHR)
  glfwInitHint(GLFW_X11_XCB_VULKAN_SURFACE, false);
#endif
  glfwInit();
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

  window_ =
      glfwCreateWindow(width, height, window_title.c_str(), nullptr, nullptr);

  setupCallback();

  // set input mode
  glfwSetInputMode(window_, GLFW_RAW_MOUSE_MOTION, GLFW_FALSE);
}

GlfwWindow::~GlfwWindow() {
  glfwDestroyWindow(window_);
  glfwTerminate();
}

bool GlfwWindow::shouldClose() { return glfwWindowShouldClose(window_); }

VkSurfaceKHR GlfwWindow::createSurface(VkInstance instance) {
  VkSurfaceKHR surface{VK_NULL_HANDLE};
  VK_THROW_IF_ERROR(
      glfwCreateWindowSurface(instance, window_, nullptr, &surface),
      "failed to create window surface");
  return surface;
}

void GlfwWindow::getWindowSize(uint32_t &width, uint32_t &height) {
  glfwGetWindowSize(window_, reinterpret_cast<int *>(&width),
                    reinterpret_cast<int *>(&height));
}

void GlfwWindow::setupCallback() {

  // init callbacks
  glfwSetWindowUserPointer(window_, this);
  glfwSetKeyCallback(window_, keyCallback);
  glfwSetCharCallback(window_, charCallback);
  glfwSetCharModsCallback(window_, charModsCallback);
  glfwSetMouseButtonCallback(window_, mouseButtonCallback);
  glfwSetCursorPosCallback(window_, cursorPosCallback);
  glfwSetCursorEnterCallback(window_, cursorEnterCallback);
  glfwSetScrollCallback(window_, scrollCallback);
  glfwSetDropCallback(window_, dropCallback);
  glfwSetWindowSizeCallback(window_, windowSizeCallback);
  glfwSetWindowCloseCallback(window_, windowCloseCallback);
}

void GlfwWindow::keyCallback(GLFWwindow *window, int key, int scancode,
                             int action, int mods) {
  g_engine.getEventSystem()->asyncDispatch(
      std::make_shared<WindowKeyEvent>(key, scancode, action, mods));
}

void GlfwWindow::charCallback(GLFWwindow *window, unsigned int codepoint) {
  g_engine.getEventSystem()->asyncDispatch(
      std::make_shared<WindowCharEvent>(codepoint));
}

void GlfwWindow::charModsCallback(GLFWwindow *window, unsigned int codepoint,
                                  int mods) {
  g_engine.getEventSystem()->asyncDispatch(
      std::make_shared<WindowCharModsEvent>(codepoint, mods));
}

void GlfwWindow::mouseButtonCallback(GLFWwindow *window, int button, int action,
                                     int mods) {
  g_engine.getEventSystem()->asyncDispatch(
      std::make_shared<WindowMouseButtonEvent>(button, action, mods));
}

void GlfwWindow::cursorPosCallback(GLFWwindow *window, double xpos,
                                   double ypos) {
  GlfwWindow *e_window = (GlfwWindow *)glfwGetWindowUserPointer(window);
  e_window->xpos_ = xpos;
  e_window->ypos_ = ypos;

  g_engine.getEventSystem()->asyncDispatch(
      std::make_shared<WindowCursorPosEvent>(xpos, ypos));
}

void GlfwWindow::cursorEnterCallback(GLFWwindow *window, int entered) {
  g_engine.getEventSystem()->asyncDispatch(
      std::make_shared<WindowCursorEnterEvent>(entered));
}

void GlfwWindow::scrollCallback(GLFWwindow *window, double xoffset,
                                double yoffset) {
  g_engine.getEventSystem()->asyncDispatch(
      std::make_shared<WindowScrollEvent>(xoffset, yoffset));
}

void GlfwWindow::dropCallback(GLFWwindow *window, int count,
                              const char **paths) {
  GlfwWindow *e_window = (GlfwWindow *)glfwGetWindowUserPointer(window);
  g_engine.getEventSystem()->asyncDispatch(std::make_shared<WindowDropEvent>(
      count, paths, e_window->xpos_, e_window->ypos_));
}

void GlfwWindow::windowSizeCallback(GLFWwindow *window, int width, int height) {
  g_engine.getEventSystem()->asyncDispatch(
      std::make_shared<WindowSizeEvent>(width, height));
}

void GlfwWindow::windowCloseCallback(GLFWwindow *window) {
  glfwSetWindowShouldClose(window, true);
}

} // namespace mango