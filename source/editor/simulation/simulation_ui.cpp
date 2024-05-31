#include "simulation_ui.h"
#include <engine/functional/component/component_camera.h>
#include <engine/functional/global/engine_context.h>
#include <engine/functional/world/world.h>
#include <engine/utils/event/event_system.h>
#include <engine/utils/base/macro.h>
// #include "editor/global/editor_context.h"

// #include "engine/core/vulkan/vulkan_rhi.h"
// #include "engine/function/render/render_system.h"
// #include "engine/utils/event/event_system.h"

// #include "engine/function/engine/component/animation_component.h"
// #include "engine/function/engine/component/animator_component.h"
// #include "engine/function/engine/component/camera_component.h"
// #include "engine/function/engine/component/directional_light_component.h"
// #include "engine/function/engine/component/skeletal_mesh_component.h"
// #include "engine/function/engine/component/sky_light_component.h"
// #include "engine/function/engine/component/spot_light_component.h"
// #include "engine/function/engine/component/static_mesh_component.h"
// #include "engine/function/engine/component/transform_component.h"
// #include "engine/function/engine/world/world_manager.h"
// #include "engine/platform/timer/timer.h"
// #include "engine/resource/asset/asset_manager.h"

#include <GLFW/glfw3.h>
#include <ImGuizmo/ImGuizmo.h>
#include <engine/functional/global/engine_context.h>
#include <engine/functional/render/render_system.h>
#include <engine/utils/vk/image.h>
#include <engine/utils/vk/sampler.h>
#include <imgui/backends/imgui_impl_vulkan.h>
#include <imgui/imgui_internal.h>

namespace mango {

void SimulationUI::init() {
  EditorUI::init();
  title_ = "Simulation";
  coordinate_mode_ = ECoordinateMode::Local;
  operation_mode_ = EOperationMode::Translate;
  mouse_right_button_pressed_ = false;

  g_engine.getEventSystem()->addListener(
      EEventType::WindowKey,
      std::bind(&SimulationUI::onKey, this, std::placeholders::_1));
  g_engine.getEventSystem()->addListener(
      EEventType::SelectEntity,
      std::bind(&SimulationUI::onSelectEntity, this, std::placeholders::_1));
}

SimulationUI::~SimulationUI() {
  ImGui_ImplVulkan_RemoveTexture(color_texture_desc_set_);
}

void SimulationUI::construct() {
  const std::string &world_name = g_engine.getWorld()->getName();
  sprintf(title_buf_, "%s %s###%s", ICON_FA_GAMEPAD, world_name.c_str(),
          title_.c_str());

  // bool is_simulating_fullscreen =
  //     g_engine.isSimulating() && g_editor.isSimulationPanelFullscreen();
  bool is_simulating_fullscreen = false;
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
  ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize,
                      is_simulating_fullscreen ? 0.0f : 1.0f);

  if (!ImGui::Begin(title_buf_)) {
    ImGui::End();
    ImGui::PopStyleVar(2);
    return;
  }
  updateWindowRegion();

  // hide window tab bar
  if (ImGui::IsWindowDocked()) {
    ImGuiWindow *window = ImGui::FindWindowByName(title_buf_);
    ImGuiDockNode *node = window->DockNode;
    bool show_tab = !is_simulating_fullscreen;
    if ((!show_tab && !node->IsHiddenTabBar()) ||
        (show_tab && node->IsHiddenTabBar())) {
      node->WantHiddenTabBarToggle = true;
    }
  }

  ImVec2 cursor_screen_pos = ImGui::GetCursorScreenPos();
  uint32_t mouse_x =
      static_cast<uint32_t>(ImGui::GetMousePos().x - cursor_screen_pos.x);
  uint32_t mouse_y =
      static_cast<uint32_t>(ImGui::GetMousePos().y - cursor_screen_pos.y);

  ImVec2 content_size = ImGui::GetContentRegionAvail();
  ImGui::Image(color_texture_desc_set_, content_size);

  // if (g_engine.isSimulating()) {
  //   updateCamera();
  //   ImGui::End();
  //   ImGui::PopStyleVar(2);
  //   return;
  // }

  ImGui::SetCursorScreenPos(cursor_screen_pos);
  ImGui::SetNextItemAllowOverlap();
  // if (ImGui::InvisibleButton("image", content_size) &&
  //     (!m_selected_entity.lock() || !ImGuizmo::IsOver())) {
  //   g_engine.getEventSystem()->syncDispatch(
  //       std::make_shared<PickEntityEvent>(mouse_x, mouse_y));
  // }
  updateCamera();

  // allow drag from asset ui
  // handleDragDropTarget(glm::vec2(mouse_x, mouse_y),
  //                     glm::vec2(content_size.x, content_size.y));

  ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.26f, 0.59f, 0.98f, 0.8f));
  ImGui::SetCursorPos(ImVec2(10, 30));
  sprintf(title_buf_, "%s view", ICON_FA_DICE_D6);
  if (ImGui::Button(title_buf_, ImVec2(64, 24))) {
    ImGui::OpenPopup("view");
  }

  static int view_index = 0;
  static const std::vector<std::string> views = {
      "perspective", "top", "bottom", "left", "right", "front", "back"};
  ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(-2.0f, -2.0f));
  ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8.0f, 8.0f));
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(12.0f, 12.0f));
  // if (constructRadioButtonPopup("view", views, view_index)) {
  //   static glm::vec3 last_camera_rotation;
  //   static glm::vec3 ortho_camera_rotations[6] = {
  //       glm::vec3(0.0f, 0.0f, -90.0f), glm::vec3(0.0f, 0.0f, 90.0f),
  //       glm::vec3(0.0f, 90.0f, 0.0f),  glm::vec3(0.0f, -90.0f, 0.0f),
  //       glm::vec3(0.0f, 180.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f)};

  //   if (view_index == 0) {
  //     m_camera_component.lock()->m_projection_type =
  //         EProjectionType::Perspective;
  //     m_camera_component.lock()->getTransformComponent()->m_rotation =
  //         last_camera_rotation;
  //   } else {
  //     if (m_camera_component.lock()->m_projection_type ==
  //         EProjectionType::Perspective) {
  //       m_camera_component.lock()->m_projection_type =
  //           EProjectionType::Orthographic;
  //       last_camera_rotation =
  //           m_camera_component.lock()->getTransformComponent()->m_rotation;
  //     }

  //     m_camera_component.lock()->getTransformComponent()->m_rotation =
  //         ortho_camera_rotations[view_index - 1];
  //   }
  // }

  ImGui::SameLine();
  sprintf(title_buf_, "%s shader", ICON_FA_BOWLING_BALL);
  if (ImGui::Button(title_buf_, ImVec2(75, 24))) {
    ImGui::OpenPopup("shader");
  }

  // static int shader_index = 0;
  // static const std::vector<std::string> shaders = {
  //     "lit",      "unlit",     "wireframe",  "lighting only",
  //     "depth",    "normal",    "base color", "emissive color",
  //     "metallic", "roughness", "occlusion",  "opacity"};
  // if (constructRadioButtonPopup("shader", shaders, shader_index)) {
  //   g_engine.renderSystem()->setShaderDebugOption(shader_index);
  // }

  ImGui::SameLine();
  sprintf(title_buf_, "%s show", ICON_FA_EYE);
  if (ImGui::Button(title_buf_, ImVec2(64, 24))) {
    ImGui::OpenPopup("show");
  }

  static std::vector<std::pair<std::string, bool>> shows = {
      {"anti-aliasing", false}, {"bounding boxes", false},
      {"collision", false},     {"grid", false},
      {"static meshes", true},  {"skeletal meshes", true},
      {"translucency", true}};
  constructCheckboxPopup("show", shows);
  ImGui::PopStyleVar(3);

  constructOperationModeButtons();
  ImGui::PopStyleColor();

  // constructImGuizmo();

  ImGui::End();
  ImGui::PopStyleVar(2);
}

// void SimulationUI::loadAsset(const std::string &url) {
//   const auto &as = g_engine.assetManager();
//   EAssetType asset_type = as->getAssetType(url);
//   std::string basename = g_engine.fileSystem()->basename(url);

//   const auto &world = g_engine.worldManager()->getCurrentWorld();
//   m_created_entity = world->createEntity(basename);

//   if (asset_type == EAssetType::StaticMesh) {
//     std::shared_ptr<StaticMeshComponent> static_mesh_component =
//         std::make_shared<StaticMeshComponent>();
//     std::shared_ptr<StaticMesh> static_mesh = as->loadAsset<StaticMesh>(url);
//     static_mesh_component->setStaticMesh(static_mesh);
//     m_created_entity->addComponent(static_mesh_component);
//   } else if (asset_type == EAssetType::SkeletalMesh) {
//     std::shared_ptr<SkeletalMeshComponent> skeletal_mesh_component =
//         std::make_shared<SkeletalMeshComponent>();
//     std::shared_ptr<SkeletalMesh> skeletal_mesh =
//         as->loadAsset<SkeletalMesh>(url);
//     skeletal_mesh_component->setSkeletalMesh(skeletal_mesh);
//     m_created_entity->addComponent(skeletal_mesh_component);

//     const auto &fs = g_engine.fileSystem();
//     std::vector<std::string> filenames =
//         fs->traverse(fs->absolute(fs->dir(url)));
//     for (const std::string &filename : filenames) {
//       URL asset_url(filename);
//       EAssetType asset_type = as->getAssetType(asset_url);
//       if (asset_type == EAssetType::Animation &&
//           !m_created_entity->hasComponent(AnimationComponent)) {
//         std::shared_ptr<AnimationComponent> animation_component =
//             std::make_shared<AnimationComponent>();
//         std::shared_ptr<Animation> animation =
//             as->loadAsset<Animation>(asset_url);
//         animation_component->addAnimation(animation);
//         m_created_entity->addComponent(animation_component);
//       } else if (asset_type == EAssetType::Skeleton &&
//                  !m_created_entity->hasComponent(AnimatorComponent)) {
//         std::shared_ptr<AnimatorComponent> animator_component =
//             std::make_shared<AnimatorComponent>();
//         std::shared_ptr<Skeleton> skeleton =
//         as->loadAsset<Skeleton>(asset_url);
//         animator_component->setTickEnabled(true);
//         animator_component->setSkeleton(skeleton);
//         m_created_entity->addComponent(animator_component);
//       }
//     }

//     m_created_entity->setTickEnabled(true);
//     m_created_entity->setTickInterval(0.0167f);
//   }
// }

bool SimulationUI::constructRadioButtonPopup(
    const std::string &popup_name, const std::vector<std::string> &values,
    int &index) {
  bool is_radio_button_pressed = false;
  ImGui::SetNextWindowPos(
      ImVec2(ImGui::GetItemRectMin().x, ImGui::GetItemRectMax().y));
  if (ImGui::BeginPopup(popup_name.c_str())) {
    for (size_t i = 0; i < values.size(); ++i) {
      if (ImGui::RadioButton(values[i].c_str(), &index, i)) {
        is_radio_button_pressed = true;
      }
      if (i != values.size() - 1) {
        ImGui::Spacing();
      }
    }
    ImGui::EndPopup();
  }

  return is_radio_button_pressed;
}

void SimulationUI::constructCheckboxPopup(
    const std::string &popup_name,
    std::vector<std::pair<std::string, bool>> &values) {
  static int show_debug_option = 0;
  ImGui::SetNextWindowPos(
      ImVec2(ImGui::GetItemRectMin().x, ImGui::GetItemRectMax().y));
  if (ImGui::BeginPopup(popup_name.c_str())) {
    for (size_t i = 0; i < values.size(); ++i) {
      if (ImGui::Checkbox(values[i].first.c_str(), &values[i].second)) {
        if (values[i].second) {
          show_debug_option |= (1 << i);
        } else {
          show_debug_option &= ~(1 << i);
        }
        // TODO: set debug option
        // g_engine.renderSystem()->setShowDebugOption(show_debug_option);
      }
      if (i != values.size() - 1) {
        ImGui::Spacing();
      }
    }
    ImGui::EndPopup();
  }
}

void SimulationUI::constructOperationModeButtons() {
  std::vector<std::string> names = {ICON_FA_MOUSE_POINTER, ICON_FA_MOVE,
                                    ICON_FA_SYNC_ALT, ICON_FA_EXPAND};
  for (size_t i = 0; i < names.size(); ++i) {
    ImGui::SameLine(i == 0 ? ImGui::GetContentRegionAvail().x - 130 : 0.0f);
    ImGui::PushStyleColor(ImGuiCol_Button,
                          i == (size_t)operation_mode_
                              ? ImVec4(0.26f, 0.59f, 0.98f, 0.8f)
                              : ImVec4(0.4f, 0.4f, 0.4f, 0.8f));
    if (ImGui::Button(names[i].c_str(), ImVec2(24, 24))) {
      operation_mode_ = (EOperationMode)i;
    }
    ImGui::PopStyleColor();
  }
}

// void SimulationUI::constructImGuizmo() {
//   if (!m_selected_entity.lock()) {
//     return;
//   }

//   // set translation/rotation/scale gizmos
//   ImGuizmo::SetID(0);
//   ImGuizmo::SetOrthographic(m_camera_component.lock()->m_projection_type ==
//                             EProjectionType::Orthographic);
//   ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y,
//                     ImGui::GetWindowWidth(), ImGui::GetWindowHeight());
//   ImGuizmo::SetDrawlist();

//   const float *p_view =
//       glm::value_ptr(m_camera_component.lock()->getViewMatrix());
//   const float *p_projection = glm::value_ptr(
//       m_camera_component.lock()->getProjectionMatrixNoYInverted());

//   auto transform_component =
//       m_selected_entity.lock()->getComponent(TransformComponent);
//   glm::mat4 matrix = transform_component->getGlobalMatrix();
//   if (m_operation_mode != EOperationMode::Pick) {
//     ImGuizmo::OPERATION operation = ImGuizmo::TRANSLATE;
//     if (m_operation_mode == EOperationMode::Rotate) {
//       operation = ImGuizmo::ROTATE;
//     } else if (m_operation_mode == EOperationMode::Scale) {
//       operation = ImGuizmo::SCALE;
//     }

//     glm::mat4 delta_matrix = glm::mat4(1.0);
//     ImGuizmo::Manipulate(p_view, p_projection, operation,
//                          (ImGuizmo::MODE)m_coordinate_mode,
//                          glm::value_ptr(matrix),
//                          glm::value_ptr(delta_matrix), nullptr, nullptr,
//                          nullptr);

//     // only set gizmo's transformation to selected entity if the simulation
//     // window is focused
//     if (ImGui::IsWindowFocused()) {
//       glm::vec3 translation, rotation, scale;
//       ImGuizmo::DecomposeMatrixToComponents(
//           glm::value_ptr(matrix), glm::value_ptr(translation),
//           glm::value_ptr(rotation), glm::value_ptr(scale));

//       if (m_operation_mode == EOperationMode::Translate) {
//         transform_component->m_position = translation;
//       } else if (m_operation_mode == EOperationMode::Rotate) {
//         transform_component->m_rotation = rotation;
//       } else if (m_operation_mode == EOperationMode::Scale) {
//         transform_component->m_scale = scale;
//       }
//     }
//   }
// }

void SimulationUI::updateCamera() {
  // auto cameras = g_engine.getWorld()->getCameras();
  // CameraComponent *default_camera = nullptr;
  // // std::shared_ptr<TransformRelationship> tr = nullptr;
  // for (auto entity : cameras) {
  //   auto name = cameras.get<std::string>(entity);
  //   if (name != "default##camera")
  //     continue;
  //   default_camera = &(cameras.get<CameraComponent>(entity));
  //   // tr = cameras.get<std::shared_ptr<TransformRelationship>>();
  // }

  // // set camera component
  // // assert(default_camera != nullptr);
  // if (default_camera != nullptr)
  //   default_camera->setAspect(content_region_.z() / content_region_.w());

  // if (g_engine.isSimulating()) {
  //   m_mouse_right_button_pressed =
  //   ImGui::IsMouseDown(ImGuiMouseButton_Right);
  // } else {
  auto &default_camera = g_engine.getWorld()->getDefaultCameraComp();
  mouse_right_button_pressed_ =
      ImGui::IsItemHovered() && ImGui::IsMouseDown(ImGuiMouseButton_Right);
  if(ImGui::IsItemHovered() && ImGui::IsMouseDragging(ImGuiMouseButton_Left))
  {
    auto delta = ImGui::GetMouseDragDelta(ImGuiMouseButton_Left);
    ImGui::ResetMouseDragDelta(ImGuiMouseButton_Left);
    // get current mouse position
    auto cur_pos = ImGui::GetMousePos();
    Eigen::Vector2i cpos(cur_pos.x, cur_pos.y);
    cpos = (cpos - content_region_.head<2>()) - (content_region_.tail<2>())/2;// -half to half
    float normalize_l = std::min(content_region_.z(), content_region_.w());
    Eigen::Vector4f pos(cpos.x()-delta.x, cpos.y()-delta.y,
                        cpos.x(), cpos.y());
    pos = (2.0f / normalize_l) * pos;
    default_camera.rotate(pos);
  }
  // }

  // m_camera_component.lock()->setInput(m_mouse_right_button_pressed,
  //                                     isFocused());
}

void SimulationUI::onWindowResize() {
  // resize render pass
  g_engine.getRenderSystem()->resize3DView(content_region_.z(),
                                           content_region_.w());
  // TODO update camera's aspect ratio
  auto &default_camera = g_engine.getWorld()->getDefaultCameraComp();
  default_camera.setAspect(content_region_.z() / content_region_.w());
  // recreate color image and view
  if (color_texture_desc_set_ != VK_NULL_HANDLE) {
    ImGui_ImplVulkan_RemoveTexture(color_texture_desc_set_);
  }
  color_texture_desc_set_ = ImGui_ImplVulkan_AddTexture(
      texture_2d_sampler_->getHandle(),
      g_engine.getRenderSystem()->getColorImageView()->getHandle(),
      VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
}

void SimulationUI::onKey(const std::shared_ptr<class Event> &event) {
  // const WindowKeyEvent *key_event =
  //     static_cast<const WindowKeyEvent *>(event.get());
  // if (key_event->action != GLFW_PRESS) {
  //   return;
  // }

  //   if (key_event->key == GLFW_KEY_F11) {
  //     g_editor.toggleFullscreen();
  //   }

  //   if (g_engine.isEditor()) {
  //     EWorldMode current_world_mode =
  //     g_engine.worldManager()->getWorldMode(); if (key_event->mods ==
  //     GLFW_MOD_ALT && key_event->key == GLFW_KEY_P) {
  //       if (current_world_mode == EWorldMode::Edit) {
  //         g_engine.worldManager()->setWorldMode(EWorldMode::Play);
  //       }
  //     } else if (key_event->key == GLFW_KEY_ESCAPE) {
  //       if (current_world_mode == EWorldMode::Play) {
  //         g_engine.worldManager()->setWorldMode(EWorldMode::Edit);
  //       }
  //     }

  //     if (m_selected_entity.lock()) {
  //       uint32_t selected_entity_id = m_selected_entity.lock()->getID();
  //       if (key_event->key == GLFW_KEY_ESCAPE ||
  //           key_event->key == GLFW_KEY_DELETE) {
  //         g_engine.getEventSystem()->syncDispatch(
  //             std::make_shared<SelectEntityEvent>(UINT_MAX));
  //       }
  //       if (key_event->key == GLFW_KEY_DELETE) {
  //         g_engine.worldManager()->getCurrentWorld()->removeEntity(
  //             selected_entity_id);
  //       }
  //     }

  //     if (!isFocused()) {
  //       return;
  //     }

  //     if (m_selected_entity.lock() || !m_mouse_right_button_pressed) {
  //       if (key_event->key == GLFW_KEY_Q) {
  //         m_operation_mode = EOperationMode::Pick;
  //       } else if (key_event->key == GLFW_KEY_W) {
  //         m_operation_mode = EOperationMode::Translate;
  //       } else if (key_event->key == GLFW_KEY_E) {
  //         m_operation_mode = EOperationMode::Rotate;
  //       } else if (key_event->key == GLFW_KEY_R) {
  //         m_operation_mode = EOperationMode::Scale;
  //       }
  //     }
  //   }
}

void SimulationUI::onSelectEntity(const std::shared_ptr<class Event> &event) {
  // const SelectEntityEvent *p_event =
  //     static_cast<const SelectEntityEvent *>(event.get());

  // if (p_event->entity_id !=
  //     m_camera_component.lock()->getParent().lock()->getID()) {
  //   const auto &current_world = g_engine.worldManager()->getCurrentWorld();
  //   m_selected_entity = current_world->getEntity(p_event->entity_id);
  // }
}

// void SimulationUI::handleDragDropTarget(const glm::vec2 &mouse_pos,
//                                         const glm::vec2 &viewport_size) {
//   if (ImGui::BeginDragDropTarget()) {
//     const ImGuiPayload *payload = nullptr;
//     ImGuiDragDropFlags flags = ImGuiDragDropFlags_AcceptBeforeDelivery |
//                                ImGuiDragDropFlags_AcceptNoPreviewTooltip;
//     if (payload = ImGui::AcceptDragDropPayload("load_asset", flags)) {
//       if (!m_created_entity) {
//         std::string url((const char *)payload->Data, payload->DataSize);
//         StopWatch stop_watch;
//         stop_watch.start();
//         loadAsset(url);
//         LOG_INFO("load asset {}, elapsed time: {}ms", url,
//         stop_watch.stopMs());
//       }
//     } else if (payload = ImGui::AcceptDragDropPayload("create_entity",
//     flags)) {
//       if (!m_created_entity) {
//         const auto &world = g_engine.worldManager()->getCurrentWorld();
//         std::string playload_str((const char *)payload->Data,
//                                  payload->DataSize);
//         std::vector<std::string> splits = StringUtil::split(playload_str,
//         "-"); const std::string &entity_category = splits[0]; const
//         std::string &entity_type = splits[1];

//         if (entity_category == "Entities") {
//           m_created_entity = world->createEntity(entity_type);
//         } else if (entity_category == "Lights") {
//           m_created_entity = world->createEntity(entity_type);
//           auto transform_component =
//               m_created_entity->getComponent(TransformComponent);

//           // add light component
//           if (entity_type.find("Directional") != std::string::npos) {
//             transform_component->setRotation(glm::vec3(0.0f, 135.0f,
//             -35.2f)); m_created_entity->addComponent(
//                 std::make_shared<DirectionalLightComponent>());
//           } else if (entity_type.find("Sky") != std::string::npos) {
//             auto sky_light_component =
//             std::make_shared<SkyLightComponent>(); auto sky_texture_cube =
//                 g_engine.assetManager()->loadAsset<TextureCube>(
//                     "asset/engine/texture/ibl/texc_cloudy.texc");
//             sky_light_component->setTextureCube(sky_texture_cube);
//             m_created_entity->addComponent(sky_light_component);
//           } else if (entity_type.find("Point") != std::string::npos) {
//             m_created_entity->addComponent(
//                 std::make_shared<PointLightComponent>());
//           } else if (entity_type.find("Spot") != std::string::npos) {
//             m_created_entity->addComponent(
//                 std::make_shared<SpotLightComponent>());
//           }
//         } else if (entity_category == "Primitives") {
//           std::string url = StringUtil::format(
//               "asset/engine/mesh/primitive/sm_%s.sm", entity_type.c_str());
//           loadAsset(url);
//         }
//       }
//     }

//     if (m_created_entity) {
//       glm::vec3 place_pos = calcPlacePos(mouse_pos, viewport_size);
//       m_created_entity->getComponent(TransformComponent)->m_position =
//           place_pos;
//     }

//     if (payload && payload->IsDelivery()) {
//       m_created_entity = nullptr;
//     }

//     ImGui::EndDragDropTarget();
//   }
// }

// glm::vec3 SimulationUI::calcPlacePos(const glm::vec2 &mouse_pos,
//                                      const glm::vec2 &viewport_size) {
//   glm::vec3 ray_origin =
//       glm::unProjectZO(glm::vec3(mouse_pos.x, mouse_pos.y, 0.0f),
//                        m_camera_component.lock()->getViewMatrix(),
//                        m_camera_component.lock()->getProjectionMatrix(),
//                        glm::vec4(0.0f, 0.0f, viewport_size.x,
//                        viewport_size.y));
//   glm::vec3 ray_target =
//       glm::unProjectZO(glm::vec3(mouse_pos.x, mouse_pos.y, 1.0f),
//                        m_camera_component.lock()->getViewMatrix(),
//                        m_camera_component.lock()->getProjectionMatrix(),
//                        glm::vec4(0.0f, 0.0f, viewport_size.x,
//                        viewport_size.y));
//   glm::vec3 ray_dir = glm::normalize(ray_target - ray_origin);
//   float t = -ray_origin.y / ray_dir.y;

//   glm::vec3 place_pos = ray_origin + ray_dir * t;
//   return place_pos;
// }

} // namespace mango