#include <engine/functional/global/engine_context.h>
#include <engine/platform/glfw_window.h>
#include <engine/utils/base/error.h>
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

// static void cursorPositionCallback(GlfwWindowApplication *window, double
// xpos,
//                                    double ypos);
// static void mouseButtonCallback(GlfwWindowApplication *window, int button,
//                                 int action, int /*mods*/);
// static void scrollCallback(GlfwWindowApplication *window, double xoffset,
//                            double yoffset);
// static void windowResizeCallback(GLFWwindow *window, int width, int height);

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
  // glfwSetCursorPosCallback(window_, cursorPositionCallback);
  // glfwSetMouseButtonCallback(window_, mouseButtonCallback);
  // glfwSetScrollCallback(window_, scrollCallback);
  // glfwSetFramebufferSizeCallback(window_, windowResizeCallback);
}

// static MouseButton translateMouseButton(int button) {
//   if (button < GLFW_MOUSE_BUTTON_6) {
//     return static_cast<MouseButton>(button);
//   }

//   return MouseButton::Unknown;
// }

// static MouseAction translateMouseAction(int action) {
//   if (action == GLFW_PRESS) {
//     return MouseAction::Down;
//   } else if (action == GLFW_RELEASE) {
//     return MouseAction::Up;
//   }

//   return MouseAction::Unknown;
// }

// static void cursorPositionCallback(GlfwWindowApplication *window, double
// xpos, double ypos) {
//   if(ImGui::GetIO().WantCaptureMouse) return;
//   if (auto *app =
//           reinterpret_cast<AppBase *>(glfwGetWindowUserPointer(window))) {
//     int width, height;
//     glfwGetWindowSize(window, &width, &height);
//     xpos /= width;
//     ypos /= height;
//     app->inputMouseEvent(std::make_shared<MouseInputEvent>(false,
//         MouseButton::Unknown, MouseAction::Move, static_cast<float>(xpos),
//         static_cast<float>(ypos)));
//   }
// }

// static void mouseButtonCallback(GlfwWindowApplication *window, int button,
// int action,
//                          int /*mods*/) {
//   if(ImGui::GetIO().WantCaptureMouse) return;
//   MouseAction mouse_action = translateMouseAction(action);

//   if (auto *app =
//           reinterpret_cast<AppBase *>(glfwGetWindowUserPointer(window))) {
//     double xpos, ypos;
//     glfwGetCursorPos(window, &xpos, &ypos);
//     int width, height;
//     glfwGetWindowSize(window, &width, &height);
//     xpos /= width;
//     ypos /= height;
//     app->inputMouseEvent(std::make_shared<MouseInputEvent>(
//         false, translateMouseButton(button), mouse_action,
//         static_cast<float>(xpos), static_cast<float>(ypos)));
//   }
// }

// static void scrollCallback(GlfwWindowApplication* window, double xoffset,
// double yoffset) {
//   if(ImGui::GetIO().WantCaptureMouse) return;
//   if (AppBase* app =
//   reinterpret_cast<AppBase*>(glfwGetWindowUserPointer(window))) {
//     app->inputMouseEvent(std::make_shared<MouseInputEvent>(false,
//         MouseButton::Unknown, MouseAction::Scroll,
//         static_cast<float>(xoffset), static_cast<float>(yoffset)));
//   }
// }

// void GlfwWindowApplication::initImgui() {
//   ImGui_ImplGlfw_InitForVulkan(window_,
//                                true); // init viewport and key/mouse events
// }

// void GlfwWindowApplication::shutdownImgui()
// {
//   ImGui_ImplGlfw_Shutdown();
// }
// void GlfwWindowApplication::imguiNewFrame()
// {
//   ImGui_ImplGlfw_NewFrame();
// }

// void GlfwWindowApplication::getExtent(uint32_t &width, uint32_t &height)
// const {
//   glfwGetWindowSize(window_, reinterpret_cast<int *>(&width),
//                     reinterpret_cast<int *>(&height));
//   width = std::max(width, 1u);
//   height = std::max(height, 1u);
// }

} // namespace mango