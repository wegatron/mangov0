#include "editor_ui.h"
#include <engine/functional/global/engine_context.h>
#include <engine/platform/glfw_window.h>
#include <engine/resource/gpu_asset_manager.hpp>
#include <engine/utils/vk/image.h>
#include <engine/utils/vk/resource_cache.h>
#include <engine/utils/vk/sampler.h>
#include <imgui/backends/imgui_impl_vulkan.h>
#include <imgui/imgui_internal.h>

namespace mango {
ImGuiImage::~ImGuiImage() { ImGui_ImplVulkan_RemoveTexture(tex_id); }

void EditorUI::init() {
  auto driver = g_engine.getDriver();
  auto resource_cache = g_engine.getResourceCache();
  texture_2d_sampler_ = resource_cache->requestSampler(
      driver, VK_FILTER_LINEAR, VK_FILTER_LINEAR, VK_SAMPLER_MIPMAP_MODE_LINEAR,
      VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
      VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE);
}

void EditorUI::updateWindowRegion() {
  int32_t new_pos_y = ImGui::GetCursorScreenPos().y;
  int32_t new_pos_x = ImGui::GetCursorScreenPos().x;
  if (content_region_.x() != new_pos_x || content_region_.y() != new_pos_y) {
    content_region_.x() = new_pos_x;
    content_region_.y() = new_pos_y;
    onWindowRepos();
  }

  ImVec2 m_new_size = ImGui::GetContentRegionAvail();
  int32_t new_width = static_cast<int32_t>(m_new_size.x);
  int32_t new_height = static_cast<int32_t>(m_new_size.y);
  if (content_region_.z() != new_width || content_region_.w() != new_height) {
    content_region_.z() = new_width;
    content_region_.w() = new_height;
    onWindowResize();
  }
}

std::shared_ptr<ImGuiImage>
EditorUI::loadImGuiImageFromFile(const std::string &filename) {
  if (imgui_images_.find(filename) != imgui_images_.end()) {
    return imgui_images_[filename];
  }

  auto driver = g_engine.getDriver();
  auto gpu_asset_manager = g_engine.getGPUAssetManager();
  std::shared_ptr<ImGuiImage> image = std::make_shared<ImGuiImage>();
  image->image_view = gpu_asset_manager->request<ImageView>(filename, nullptr);
  image->sampler = texture_2d_sampler_;
  image->tex_id = ImGui_ImplVulkan_AddTexture(
      image->sampler->getHandle(), image->image_view->getHandle(),
      VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
  imgui_images_[filename] = image;
  return image;
}

// std::shared_ptr<ImGuiImage> EditorUI::loadImGuiImageFromTexture2D(
//     std::shared_ptr<class Texture2D> &texture) {
//   std::shared_ptr<ImGuiImage> image = std::make_shared<ImGuiImage>();
//   image->image_view_sampler = texture->m_image_view_sampler;
//   image->tex_id = ImGui_ImplVulkan_AddTexture(
//       m_texture_2d_sampler, image->image_view_sampler.view,
//       VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
//   image->is_owned = false;
//   m_imgui_images[texture->getURL()] = image;

//   return image;
// }

std::shared_ptr<mango::ImGuiImage> EditorUI::loadImGuiImageFromImageViewSampler(
    const std::shared_ptr<ImageView> &image_view,
    const std::shared_ptr<Sampler> &sampler) {
  std::shared_ptr<ImGuiImage> image = std::make_shared<ImGuiImage>();
  image->image_view = image_view;
  image->sampler = sampler;
  image->tex_id =
      ImGui_ImplVulkan_AddTexture(sampler->getHandle(), image_view->getHandle(),
                                  VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
  uint64_t hash_id = reinterpret_cast<uint64_t>(image_view->getHandle());
  std::string hash_str = std::to_string(hash_id);
  imgui_images_[hash_str] = image;
  return image;
}

std::shared_ptr<mango::ImGuiImage>
EditorUI::getImGuiImageFromCache(const std::string &url) {
  return imgui_images_[url];
}

ImFont *EditorUI::defaultFont() { return ImGui::GetIO().Fonts->Fonts[0]; }

ImFont *EditorUI::smallFont() { return ImGui::GetIO().Fonts->Fonts[1]; }

ImFont *EditorUI::bigIconFont() { return ImGui::GetIO().Fonts->Fonts[2]; }

bool EditorUI::isFocused() { return !isPoppingUp() && isMouseFocused(); }

bool EditorUI::isPoppingUp() {
  ImGuiContext &g = *GImGui;
  return !g.OpenPopupStack.empty();
}

bool EditorUI::isImGuiImageLoaded(const std::string &url) {
  return imgui_images_.find(url) != imgui_images_.end();
}

bool EditorUI::isMouseFocused() {
  int32_t xpos = 0;
  int32_t ypos = 0;
  g_engine.getWindow()->getMousePos(xpos, ypos);
  return xpos > content_region_.x() &&
         xpos < content_region_.x() + content_region_.z() &&
         ypos > content_region_.y() &&
         ypos < content_region_.y() + content_region_.w();
}

} // namespace mango