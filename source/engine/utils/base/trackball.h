#pragma once
#include <Eigen/Dense>

namespace mango {
void trackballRotate(Eigen::Matrix4f &view_mat, const Eigen::Vector4f &normalized_mouse_pos);
  
  //void pan(const Eigen::Vector2f &prev_mouse_pos, const Eigen::Vector2f &cur_mouse_pos);
  
  //void zoom(const float delta_y);  
} // namespace vk_engine