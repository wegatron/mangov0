#include <engine/functional/global/engine_context.h>
#include <engine/platform/glfw_window.h>
#include <engine/utils/base/error.h>
#include <engine/utils/vk/framebuffer.h>
#include <engine/utils/vk/image.h>
#include <engine/utils/vk/swapchain.h>
// #include <engine/app_template/app_base.h>
// #include <imgui/backends/imgui_impl_glfw.h>
// #include <imgui/imgui.h>
// #include <imgui/backends/imgui_impl_vulkan.h>

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
  ds_format_ = VK_FORMAT_D24_UNORM_S8_UINT;
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

// std::vector<std::shared_ptr<RenderTarget>> GlfwWindow::createRenderTargets()
// {
//   uint32_t width = 0;
//   uint32_t height = 0;
//   glfwGetWindowSize(window_, reinterpret_cast<int*>(&width),
//   reinterpret_cast<int*>(&height)); VkExtent3D extent{width, height, 1};

//   const auto img_cnt = swapchain_->getImageCount();
//   std::vector<std::shared_ptr<RenderTarget>> rts(img_cnt);
//   auto driver = g_engine.getDriver();
//   for (uint32_t i = 0; i < img_cnt; ++i) {
//     auto depth_image = std::make_shared<Image>(
//         driver, 0,
//         ds_format_, extent, VK_SAMPLE_COUNT_1_BIT,
//         VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
//         VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE);
//     auto depth_img_view = std::make_shared<ImageView>(
//         depth_image, VK_IMAGE_VIEW_TYPE_2D, ds_format_,
//         VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT, 0, 0, 1, 1);
//     rts[i] = std::make_shared<RenderTarget>(
//       std::initializer_list<std::shared_ptr<ImageView>>{swapchain_->getImageView(i),
//       depth_img_view},
//       std::initializer_list<VkFormat>{swapchain_->getImageFormat()},
//       ds_format_, extent.width,
//       extent.height, 1u);
//   }
//   return rts;
// }

// static void cursorPositionCallback(GlfwWindowApplication *window, double
// xpos,
//                                    double ypos);
// static void mouseButtonCallback(GlfwWindowApplication *window, int button,
//                                 int action, int /*mods*/);
// static void scrollCallback(GlfwWindowApplication *window, double xoffset,
//                            double yoffset);
// static void windowResizeCallback(GLFWwindow *window, int width, int height);

void GlfwWindow::setupCallback() {
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