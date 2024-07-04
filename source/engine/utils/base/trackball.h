#pragma once
#include <Eigen/Dense>

namespace mango {

void doRotate(Eigen::Matrix4f &view_mat, const Eigen::Vector4f &normalized_mouse_pos);
  
void doPan(Eigen::Matrix4f &view_mat, const float aspect, const float dis, const Eigen::Vector4f &normalized_mouse_pos);

void doZoom(Eigen::Matrix4f &view_mat, const float dis, const float delta);

} // namespace vk_engine