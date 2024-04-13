#include "asset_ui.h"
#include <engine/asset/texture.h>
#include <engine/utils/base/macro.h>
#include <engine/utils/base/string_util.h>
#include <engine/utils/base/timer.h>
#include <engine/utils/event/event_system.h>
#include <queue>

namespace mango {
void AssetUI::init() {
  EditorUI::init();
  title_ = "Asset";

  // set poll folder timer
  const float k_poll_folder_time = 1.0f;
  m_poll_folder_timer_handle = g_engine.getTimerManager()->addTimer(
      k_poll_folder_time, [this]() { pollFolders(); }, true, true);
  openFolder(g_engine.getFileSystem()->getAssetDir());

  // load icon images
  asset_images_[EAssetType::INVALID] =
      loadImGuiImageFromFile("asset/engine/texture/ui/invalid.png");
  asset_images_[EAssetType::TEXTURE2D] =
      loadImGuiImageFromFile("asset/engine/texture/ui/texture_2d.png");
  asset_images_[EAssetType::TEXTURECUBE] =
      loadImGuiImageFromFile("asset/engine/texture/ui/texture_cube.png");
  asset_images_[EAssetType::MATERIAL] =
      loadImGuiImageFromFile("asset/engine/texture/ui/material.png");
  asset_images_[EAssetType::SKELETON] =
      loadImGuiImageFromFile("asset/engine/texture/ui/skeleton.png");
  asset_images_[EAssetType::STATICMESH] =
      loadImGuiImageFromFile("asset/engine/texture/ui/static_mesh.png");
  asset_images_[EAssetType::SKELETALMESH] =
      loadImGuiImageFromFile("asset/engine/texture/ui/skeletal_mesh.png");
  asset_images_[EAssetType::ANIMATION] =
      loadImGuiImageFromFile("asset/engine/texture/ui/animation.png");
  asset_images_[EAssetType::WORLD] =
      loadImGuiImageFromFile("asset/engine/texture/ui/world.png");
  empty_folder_image_ =
      loadImGuiImageFromFile("asset/engine/texture/ui/empty_folder.png");
  non_empty_folder_image_ =
      loadImGuiImageFromFile("asset/engine/texture/ui/non_empty_folder.png");

  // register drop callback
  g_engine.getEventSystem()->addListener(
      EEventType::WindowDrop,
      std::bind(&AssetUI::onDropFiles, this, std::placeholders::_1));
}

void AssetUI::construct() {
  // draw asset widget
  sprintf(title_buf_, "%s %s###%s", ICON_FA_FAN, title_.c_str(),
          title_.c_str());
  if (!ImGui::Begin(title_buf_)) {
    m_imported_files.clear();
    ImGui::End();
    return;
  }

  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(2.0f, 2.0f));
  ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4.0f, 4.0f));

  const float k_folder_tree_width_scale = 0.2f;
  const uint32_t k_spacing = 4;
  ImVec2 content_size = ImGui::GetContentRegionAvail();
  content_size.x -= k_spacing;

  // folder tree
  ImGui::BeginChild(
      "folder_tree",
      ImVec2(content_size.x * k_folder_tree_width_scale, content_size.y), true);
  ImGui::Spacing();
  constructFolderTree();
  ImGui::EndChild();

  ImGui::BeginChild("folder_tree");
  if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
    if (is_folder_tree_hovered) {
      ImGui::OpenPopup("folder_op_tree_hovered_popups");
    } else {
      ImGui::OpenPopup("folder_op_background_hovered_popups");
    }
  }
  constructFolderOpPopups("folder_op_background_hovered_popups");
  constructFolderOpPopups("folder_op_tree_hovered_popups", true);
  constructFolderOpPopupModal(m_selected_folder);
  ImGui::EndChild();

  ImGui::SameLine();

  //// folder files
  ImGui::BeginChild(
      "folder_files",
      ImVec2(content_size.x * (1 - k_folder_tree_width_scale), content_size.y),
      true);

  ImGui::Spacing();
  ImGui::Indent(k_spacing);

  // 导航栏
  ImGui::BeginChild(
      "asset_navigator",
      ImVec2(content_size.x * (1 - k_folder_tree_width_scale) - k_spacing * 3,
             24),
      true);
  constructAssetNavigator();
  ImGui::EndChild();

  // 显示文件列表
  ImGui::BeginChild("folder_files");
  ImGui::Indent(k_spacing);
  ImGui::PushFont(smallFont());
  constructFolderFiles();
  ImGui::PopFont();
  ImGui::EndChild();

  ImGui::BeginChild("folder_files");
  if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right) &&
      !is_asset_hovered) {
    ImGui::OpenPopup("folder_op_background_hovered_popups");
  }
  constructFolderOpPopups("folder_op_background_hovered_popups");
  constructFolderOpPopups("folder_op_dir_hovered_popups", true);
  constructFolderOpPopupModal(m_selected_file);
  ImGui::EndChild();
  ImGui::EndChild();

  // get folder window rect
  folder_rect_.x() = ImGui::GetItemRectMin().x;
  folder_rect_.y() = ImGui::GetItemRectMax().x;
  folder_rect_.z() = ImGui::GetItemRectMin().y;
  folder_rect_.w() = ImGui::GetItemRectMax().y;

  ImGui::PopStyleVar();
  ImGui::PopStyleVar();
  ImGui::End(); // end of foler files full

  // construct popup modal windows
  // constructImportPopups();

  // reset bool status
  is_folder_tree_hovered = false;
  is_asset_hovered = false;
}

AssetUI::~AssetUI() {
  g_engine.getTimerManager()->removeTimer(m_poll_folder_timer_handle);
}

void AssetUI::constructAssetNavigator() {
  ImVec2 button_size(20, 20);
  ImGui::Button(ICON_FA_ARROW_LEFT, button_size);

  ImGui::SameLine();
  ImGui::Button(ICON_FA_ARROW_RIGHT, button_size);

  ImGui::SameLine();
  static char str1[128] = "";
  ImGui::PushItemWidth(200.0f);
  ImGui::InputTextWithHint("##search_asset",
                           (std::string(ICON_FA_SEARCH) + " Search...").c_str(),
                           str1, IM_ARRAYSIZE(str1));
  ImGui::PopItemWidth();

  ImGui::SameLine();
  ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 10.0f);
  ImGui::Text(m_formatted_selected_folder.c_str());

  ImGui::SameLine(ImGui::GetWindowWidth() - 22);
  if (ImGui::Button(ICON_FA_COG, button_size)) {
    ImGui::OpenPopup("asset settings");
  }

  if (ImGui::BeginPopup("asset settings")) {
    if (ImGui::Checkbox("show engine assets", &show_engine_assets)) {
      pollFolders();
    }
    ImGui::EndPopup();
  }
}

void AssetUI::constructFolderFiles() {
  ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 5.0f);

  ImVec2 icon_size(80, 80);
  ImGuiStyle &style = ImGui::GetStyle();
  float max_pos_x =
      ImGui::GetWindowPos().x + ImGui::GetWindowContentRegionMax().x;

  // 当前滚动位置和可视区域
  float clip_rect_min_y = ImGui::GetCursorScreenPos().y + ImGui::GetScrollY();
  float clip_rect_max_y = clip_rect_min_y + ImGui::GetContentRegionAvail().y;

  constructAssetFilePopups();

  ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(15.0f, 24.0f));
  for (size_t i = 0; i < m_selected_files.size(); ++i) {
    bool is_clipping = false;
    HoverState &hover_state = m_selected_file_hover_states[m_selected_files[i]];
    if (hover_state.rect_min.y != 0.0f && hover_state.rect_max.y != 0.0f) {
      // 已经创建过了, 判断是否在可视区域
      if (hover_state.rect_max.y < clip_rect_min_y ||
          hover_state.rect_min.y > clip_rect_max_y) {
        is_clipping = true;
      }
    }

    if (!is_clipping) {
      constructAsset(m_selected_files[i], icon_size); // 创建asset图标
    } else {
      ImGui::Dummy(icon_size);
    }

    // 更新hover区域
    hover_state.rect_min = ImGui::GetItemRectMin();
    hover_state.rect_max = ImGui::GetItemRectMax();

    float current_pos_x = ImGui::GetItemRectMax().x;
    float next_pos_x = current_pos_x + style.ItemSpacing.x + icon_size.x;

    // 如果大于最大宽度, 换行
    if (i < m_selected_files.size() - 1 && next_pos_x < max_pos_x) {
      ImGui::SameLine();
    }
  }
  ImGui::PopStyleVar();
}

void AssetUI::constructAsset(const std::string &filename, const ImVec2 &size) {
  ImTextureID tex_id = nullptr;
  std::string basename = g_engine.getFileSystem()->basename(filename);

  const auto &asset_manager = g_engine.getAssetManager();

  if (g_engine.getFileSystem()->isFile(filename)) {
    EAssetType asset_type = asset_manager->getAssetType(filename);
    tex_id = asset_images_[asset_type]->tex_id;
    if (asset_type == EAssetType::TEXTURE2D) {
      if (isImGuiImageLoaded(filename)) {
        tex_id = getImGuiImageFromCache(filename)->tex_id;
      } else {
        auto imgui_image = loadImGuiImageFromFile(filename);
        tex_id = imgui_image->tex_id;
      }
    }
  } else if (g_engine.getFileSystem()->isDir(filename)) {
    bool is_empty = g_engine.getFileSystem()->isEmptyDir(filename);
    tex_id = is_empty ? empty_folder_image_->tex_id
                      : non_empty_folder_image_->tex_id;
  } else {
    return;
  }

  ImGui::BeginGroup();

  // draw hovered/selected background rect
  HoverState &hover_state = m_selected_file_hover_states[filename];
  bool is_hovered = hover_state.is_hovered;
  bool is_selected = m_selected_file == filename;
  if (is_hovered || is_selected) {
    ImVec4 color = ImVec4(50, 50, 50, 255);
    if (!is_hovered && is_selected) {
      color = ImVec4(0, 112, 224, 255);
    } else if (is_hovered && is_selected) {
      color = ImVec4(14, 134, 255, 255);
    }

    ImDrawFlags draw_flags = ImDrawFlags_RoundCornersBottom;
    const float k_margin = 4;
    ImGui::GetWindowDrawList()->AddRectFilled(
        ImVec2(hover_state.rect_min.x - k_margin,
               hover_state.rect_min.y - k_margin),
        ImVec2(hover_state.rect_max.x + k_margin,
               hover_state.rect_max.y + k_margin),
        IM_COL32(color.x, color.y, color.z, color.w), 3.0f, draw_flags);
  }

  // draw image
  ImGui::Image(tex_id, size);

  // draw asset name text
  ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 20.0f);
  float text_width = ImGui::CalcTextSize(basename.c_str()).x;

  if (is_renaming && m_selected_file == filename) {
    is_renaming = rename(filename, size);
  } else if (text_width > size.x) {
    ImGui::PushTextWrapPos(ImGui::GetCursorPos().x + size.x);
    ImGui::Text(basename.c_str());
    ImGui::PopTextWrapPos();
  } else {
    ImGui::SetCursorPosX(ImGui::GetCursorPos().x +
                         (size.x - text_width) * 0.5f);
    ImGui::Text(basename.c_str());
  }
  ImGui::EndGroup();

  // update asset hover and selection status
  is_asset_hovered |= hover_state.is_hovered = ImGui::IsItemHovered();
  if (ImGui::IsItemClicked(ImGuiMouseButton_Left) ||
      ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
    m_selected_file = filename;
  }

  // set drag source
  if (g_engine.getFileSystem()->isFile(filename)) {
    EAssetType asset_type = g_engine.getAssetManager()->getAssetType(filename);
    if ((asset_type == EAssetType::STATICMESH ||
         asset_type == EAssetType::SKELETALMESH) &&
        ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID |
                                   ImGuiDragDropFlags_SourceNoPreviewTooltip)) {
      std::string ref_filename = g_engine.getFileSystem()->relative(filename);
      ImGui::SetDragDropPayload("load_asset", ref_filename.data(),
                                ref_filename.size());
      ImGui::EndDragDropSource();
    }
  }

  onAssetRightClick(filename);
}

void AssetUI::constructImportPopups() {
  //  if (m_imported_files.empty()) {
  //    return;
  //  }
  //
  //  const auto &as = g_engine.getAssetManager();
  //  std::string import_folder =
  //      g_engine.getFileSystem()->relative(m_selected_folder);
  //  for (auto iter = m_imported_files.begin(); iter !=
  //  m_imported_files.end();) {
  //    // check import file type
  //    const std::string &import_file = *iter;
  //    if (as->isGltfFile(import_file)) {
  //      ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(),
  //                              ImGuiCond_Always, ImVec2(0.5f, 0.5f));
  //      ImGui::OpenPopup("Import Asset");
  //      if (ImGui::BeginPopupModal("Import Asset", nullptr,
  //                                 ImGuiWindowFlags_AlwaysAutoResize |
  //                                     ImGuiWindowFlags_NoMove)) {
  //        ImGui::Text("Importing gltf: %s", import_file.c_str());
  //        ImGui::Separator();
  //
  //        ImGui::SeparatorText("Mesh");
  //        static bool force_static_mesh = false;
  //        ImGui::Checkbox("force static mesh", &force_static_mesh);
  //
  //        static bool combine_meshes = true;
  //        ImGui::Checkbox("combine meshes", &combine_meshes);
  //
  //        ImGui::SeparatorText("Material");
  //        static bool contains_occlusion_channel = true;
  //        ImGui::Checkbox("contain occlusion channel",
  //                        &contains_occlusion_channel);
  //
  //        if (ImGui::Button("OK", ImVec2(120, 0))) {
  //          ImGui::CloseCurrentPopup();
  //
  //          StopWatch stop_watch;
  //          stop_watch.start();
  //
  //          as->importGltf(
  //              import_file, import_folder,
  //              {combine_meshes, force_static_mesh,
  //              contains_occlusion_channel});
  //          LOGI("import 3d file {} to {}, elapsed time: {}ms", import_file,
  //               import_folder, stop_watch.stopMs());
  //          iter = m_imported_files.erase(iter);
  //        }
  //
  //        ImGui::SameLine();
  //
  //        if (ImGui::Button("Cancel", ImVec2(120, 0))) {
  //          ImGui::CloseCurrentPopup();
  //          iter = m_imported_files.erase(iter);
  //        }
  //        ImGui::EndPopup();
  //      }
  //      break;
  //    } else if (as->isTexture2DFile(import_file)) {
  //      StopWatch stop_watch;
  //      stop_watch.start();
  //
  //      as->importTexture2D(import_file, import_folder);
  //      LOGI("import texture 2d {} to {}, elapsed time: {}ms", import_file,
  //           import_folder, stop_watch.stopMs());
  //      iter = m_imported_files.erase(iter);
  //    } else if (as->isTextureCubeFile(import_file)) {
  //      StopWatch stop_watch;
  //      stop_watch.start();
  //
  //      as->importTextureCube(import_file, import_folder);
  //      LOGI("import texture cube {} to {}, elapsed time: {}ms", import_file,
  //           import_folder, stop_watch.stopMs());
  //      iter = m_imported_files.erase(iter);
  //    } else {
  //      LOGW(("unknown asset format: {}", import_file);
  //      iter = m_imported_files.erase(iter);
  //    }
  //  }
}

void AssetUI::constructAssetFilePopups() {
  //  // right click option
  //  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {2.0f, 8.0f});
  //  ImGui::PushStyleColor(ImGuiCol_PopupBg, {0.2f, 0.2f, 0.2f, 1.0f});
  //  ImGui::PushFont(defaultFont());
  //  if (ImGui::BeginPopup("AssetPopups")) {
  //    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, {8.0f, 8.0f});
  //    createCustomSeperatorText("COMMON");
  //    if (ImGui::MenuItem("  Edit")) {
  //      LOGI("Edit");
  //    }
  //    if (ImGui::MenuItem("  Delete")) {
  //      LOGI("Delete");
  //    }
  //    if (ImGui::MenuItem("  Export")) {
  //      LOGI("Export");
  //    }
  //    ImGui::Separator();
  //
  //    createCustomSeperatorText("EXPLORE");
  //    if (ImGui::MenuItem("  Show in Explorer")) {
  //      LOGI("Show in Explorer");
  //    }
  //    ImGui::Separator();
  //
  //    createCustomSeperatorText("REFERENCES");
  //    if (ImGui::MenuItem("  Copy URL")) {
  //      LOGI("Copy URL");
  //    }
  //    if (ImGui::MenuItem("  Copy File Path")) {
  //      LOGI("Copy file path");
  //    }
  //    ImGui::PopStyleVar();
  //    ImGui::EndPopup();
  //  }
  //  ImGui::PopFont();
  //  ImGui::PopStyleColor();
  //  ImGui::PopStyleVar();
}

void AssetUI::constructFolderOpPopups(const std::string &str_id,
                                      bool is_background_not_hoverd) {
  bool is_delete_folder = false;

  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {2.0f, 8.0f});
  ImGui::PushStyleColor(ImGuiCol_PopupBg, {0.2f, 0.2f, 0.2f, 1.0f});
  if (ImGui::BeginPopup(str_id.c_str())) {
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, {8.0f, 8.0f});
    createCustomSeperatorText("FOLDER");
    if (ImGui::MenuItem("  New Folder")) {
      std::string new_folder = createFolder();
      is_renaming = true;
      m_selected_file = new_folder;
    }

    if (is_background_not_hoverd) {
      if (ImGui::MenuItem("  Delete")) {
        is_delete_folder = true;
      }
      if (ImGui::MenuItem("  Rename")) {
        is_renaming = true;
      }
    }

    ImGui::PopStyleVar();
    ImGui::EndPopup();
  }
  ImGui::PopStyleColor();
  ImGui::PopStyleVar();

  if (is_delete_folder) {
    ImGui::OpenPopup("Delete?");
  }
}

void AssetUI::constructFolderOpPopupModal(const std::string &path) {
  ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(),
                          ImGuiCond_Always, ImVec2(0.5f, 0.5f));
  if (ImGui::BeginPopupModal("Delete?", NULL,
                             ImGuiWindowFlags_AlwaysAutoResize |
                                 ImGuiWindowFlags_NoMove)) {
    std::string text = " Do you really want to delete " +
                       g_engine.getFileSystem()->relative(path) + "? ";
    ImGui::Text(text.c_str());
    ImGui::Separator();

    float current_width = ImGui::GetWindowWidth();
    ImVec2 button_size{current_width / 2 - 3.5f, 0.0f};

    if (ImGui::Button("Yes", button_size)) {
      deleteFolder(path);
      ImGui::CloseCurrentPopup();
    }
    ImGui::SetItemDefaultFocus();
    ImGui::SameLine(current_width / 2 + 2.0f);

    if (ImGui::Button("No", button_size)) {
      ImGui::CloseCurrentPopup();
    }
    ImGui::EndPopup();
  }

  if (ImGui::BeginPopupModal("Create?", NULL,
                             ImGuiWindowFlags_AlwaysAutoResize)) {
    ImGui::EndPopup();
  }
}

void AssetUI::openFolder(std::string folder) {
  if (!g_engine.getFileSystem()->exists(m_selected_folder)) {
    folder = g_engine.getFileSystem()->getAssetDir();
  }

  if (!folder.empty() && m_selected_folder != folder) {
    m_selected_folder = folder;

    m_formatted_selected_folder =
        g_engine.getFileSystem()->relative(m_selected_folder);
    StringUtil::replace_all(m_formatted_selected_folder, "/",
                            std::string(" ") + ICON_FA_ANGLE_RIGHT + " ");
  }

  if (!m_selected_folder.empty()) {
    m_selected_files.clear();
    const auto &iter =
        std::find_if(m_folder_nodes.begin(), m_folder_nodes.end(),
                     [this](const FolderNode &folder_node) {
                       return folder_node.dir == m_selected_folder;
                     });
    for (uint32_t child_folder : iter->child_folders) {
      m_selected_files.push_back(m_folder_nodes[child_folder].dir);
    }
    m_selected_files.insert(m_selected_files.end(), iter->child_files.begin(),
                            iter->child_files.end());
    for (const std::string &selected_file : m_selected_files) {
      if (m_selected_file_hover_states.find(selected_file) ==
          m_selected_file_hover_states.end()) {
        m_selected_file_hover_states[selected_file] = {false};
      }
    }
  }
}

void AssetUI::onDropFiles(const std::shared_ptr<class Event> &event) {
  const WindowDropEvent *drop_event =
      static_cast<const WindowDropEvent *>(event.get());
  if (drop_event->xpos < folder_rect_.x() ||
      drop_event->xpos > folder_rect_.y() ||
      drop_event->ypos < folder_rect_.z() ||
      drop_event->ypos > folder_rect_.w()) {
    return;
  }

  m_imported_files = drop_event->filenames;
}

void AssetUI::onAssetRightClick(const std::string &filename) {
  if (ImGui::IsItemHovered()) {
    if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
      if (g_engine.getFileSystem()->isDir(filename)) {
        openFolder(filename);
      } else {
        std::string basename = g_engine.getFileSystem()->basename(filename);
        LOGI("open asset {}", basename);
      }
    } else if (ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
      if (g_engine.getFileSystem()->isDir(filename)) {
        // Dir right-click event
        ImGui::OpenPopup("folder_op_dir_hovered_popups");
      } else {
        ImGui::OpenPopup("AssetPopups");
      }
    }
  }
}

void AssetUI::createCustomSeperatorText(const std::string &text) {
  ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.6f, 0.6f, 0.6f, 1.0f));
  ImGui::PushStyleVar(ImGuiStyleVar_SeparatorTextPadding, ImVec2(0.0f, 0.0f));
  ImGui::PushStyleVar(ImGuiStyleVar_SeparatorTextBorderSize, 0.0f);
  ImGui::PushFont(smallFont());
  ImGui::SeparatorText(text.c_str());
  ImGui::PopFont();
  ImGui::PopStyleVar(2);
  ImGui::PopStyleColor();
}
} // namespace mango