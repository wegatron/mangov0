#include "trackball.h"
#include <iostream>

namespace mango {

// according to https://www.khronos.org/opengl/wiki/Object_Mouse_Trackball
Eigen::Vector3f tbc(const Eigen::Vector2f &v) {
  float l = v.norm();
  float z = l < 0.5f ? sqrt(1.0f - l * l) : 0.5f / l;
  return Eigen::Vector3f(v.x(), v.y(), z);
}

void doRotate(Eigen::Matrix4f &view_mat,
                     const Eigen::Vector4f &normalized_mouse_pos) {
  Eigen::Vector3f prev_tbc = tbc(normalized_mouse_pos.head<2>());
  Eigen::Vector3f cur_tbc = tbc(normalized_mouse_pos.tail<2>());
  Eigen::Vector3f xp = prev_tbc.cross(cur_tbc);
  float xp_len = xp.norm();
  if (xp_len > 0.0f) {
    float angle = asin(xp_len);    
    Eigen::Vector3f axis = xp / xp_len;
    Eigen::AngleAxisf rotate(angle, axis);
    // update camera
    Eigen::Matrix3f r = view_mat.block<3, 3>(0, 0);
    view_mat.block<3, 3>(0, 0) = rotate * r;
  }
}

void doPan(Eigen::Matrix4f &view_mat, const float aspect, const float dis,
    const Eigen::Vector4f &normalized_mouse_pos)
{
  Eigen::Vector2f normalized_shift = normalized_mouse_pos.tail<2>() - normalized_mouse_pos.head<2>();
  Eigen::Vector2f shift =
      normalized_shift.array() * Eigen::Array2f(aspect, 1.0f) * dis * 0.1f;
  view_mat.block<2, 1>(0, 3) += shift;
}

void doZoom(Eigen::Matrix4f &view_mat, const float dis, const float delta)
{
    Eigen::Vector3f offset = dis * delta * Eigen::Vector3f(0, 0, -1);
    view_mat.block<3, 1>(0, 3) -= offset;  
}
} // namespace mango
