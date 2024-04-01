#include "editor_ui.h"
#include <engine/functional/global/engine_context.h>
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
  uint32_t new_pos_x = ImGui::GetCursorScreenPos().x;
  uint32_t new_pos_y = ImGui::GetCursorScreenPos().y;
  if (content_region_.x() != new_pos_x || content_region_.y() != new_pos_y) {
    content_region_.x() = new_pos_x;
    content_region_.y() = new_pos_y;
    onWindowRepos();
  }

  ImVec2 m_new_size = ImGui::GetContentRegionAvail();
  uint32_t new_width = static_cast<uint32_t>(m_new_size.x);
  uint32_t new_height = static_cast<uint32_t>(m_new_size.y);
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

// std::shared_ptr<mango::ImGuiImage>
// EditorUI::loadImGuiImageFromImageViewSampler(
//     const VmaImageViewSampler &image_view_sampler) {
//   std::shared_ptr<ImGuiImage> image = std::make_shared<ImGuiImage>();
//   image->image_view_sampler = image_view_sampler;
//   image->tex_id = ImGui_ImplVulkan_AddTexture(
//       image->image_view_sampler.sampler, image->image_view_sampler.view,
//       VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
//   image->is_owned = false;

//   uint64_t hash_id = reinterpret_cast<uint64_t>(image_view_sampler.view);
//   std::string hash_str = std::to_string(hash_id);
//   m_imgui_images[hash_str] = image;

//   return image;
// }

// std::shared_ptr<mango::ImGuiImage>
// EditorUI::getImGuiImageFromCache(const URL &url) {
//   return m_imgui_images[url];
// }

// ImFont *EditorUI::defaultFont() { return ImGui::GetIO().Fonts->Fonts[0]; }

// ImFont *EditorUI::smallFont() { return ImGui::GetIO().Fonts->Fonts[1]; }

// ImFont *EditorUI::bigIconFont() { return ImGui::GetIO().Fonts->Fonts[2]; }

// bool EditorUI::isFocused() { return !isPoppingUp() && isMouseFocused(); }

// bool EditorUI::isPoppingUp() {
//   ImGuiContext &g = *GImGui;
//   return !g.OpenPopupStack.empty();
// }

// bool EditorUI::isImGuiImageLoaded(const URL &url) {
//   return m_imgui_images.find(url) != m_imgui_images.end();
// }

// bool EditorUI::isMouseFocused() {
//   int xpos, ypos;
//   g_engine.windowSystem()->getMousePos(xpos, ypos);
//   return xpos > m_content_region.x &&
//          xpos < m_content_region.x + m_content_region.z &&
//          ypos > m_content_region.y &&
//          ypos < m_content_region.y + m_content_region.w;
// }

} // namespace mango