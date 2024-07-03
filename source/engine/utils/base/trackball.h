#pragma once
#include <Eigen/Dense>

namespace mango {

void doRotate(Eigen::Matrix4f &view_mat, const Eigen::Vector4f &normalized_mouse_pos);
  
void doPan(Eigen::Matrix4f &view_mat, const Eigen::Vector4f &normalized_mouse_pos);

void doZoom(Eigen::Matrix4f &view_mat, const float dis, const float delta)
{
    Eigen::Vector3f offset = dis * delta * Eigen::Vector3f(0, 0, -1);
    view_mat.block<3, 1>(0, 3) += offset;  
}

} // namespace vk_engine