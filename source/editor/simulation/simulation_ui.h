#pragma once

#include <Eigen/Dense>
#include <editor/base/editor_ui.h>

namespace mango {
enum class EOperationMode { Pick, Translate, Rotate, Scale };

enum class ECoordinateMode { Local, World };

class SimulationUI final : public EditorUI {
public:
  SimulationUI() = default;
  ~SimulationUI() override;
  virtual void init() override;
  virtual void construct() override;
  virtual void onWindowResize() override;

private:
  void loadAsset(const std::string &url);
  bool constructRadioButtonPopup(const std::string &popup_name,
                                 const std::vector<std::string> &values,
                                 int &index);
  void
  constructCheckboxPopup(const std::string &popup_name,
                         std::vector<std::pair<std::string, bool>> &values);
  void constructOperationModeButtons();
  // void constructImGuizmo();

  void onKey(const std::shared_ptr<class Event> &event);
  void onSelectEntity(const std::shared_ptr<class Event> &event);

  void updateCamera();
  // void handleDragDropTarget(const Eigen::Vector2i &mouse_pos,
  //                           const Eigen::Vector2i &viewport_size);
  // glm::vec3 calcPlacePos(const glm::vec2 &mouse_pos,
  //                        const glm::vec2 &viewport_size);

  VkDescriptorSet color_texture_desc_set_{VK_NULL_HANDLE};

  ECoordinateMode coordinate_mode_;
  EOperationMode operation_mode_;
  bool mouse_right_button_pressed_;
  // std::weak_ptr<class CameraComponent> camera_component_;
  // std::shared_ptr<class Entity> created_entity_;
  // std::weak_ptr<class Entity> selected_entity_;
};
} // namespace mango