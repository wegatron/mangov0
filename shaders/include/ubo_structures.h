#ifndef SHADERS_UBO_STRUCTURES_H
#define SHADERS_UBO_STRUCTURES_H

#include <shaders/include/constant.h>

#ifdef __cplusplus
#include <Eigen/Dense>

using vec3 = Eigen::Vector3f;
using vec4 = Eigen::Vector4f;
using mat4 = Eigen::Matrix4f;
using mat3 = Eigen::Matrix3f;
using uint = uint32_t;
#endif // __cplusplus

struct DirectionalLight {
  vec3 direction;
  int cast_shadow;
  vec3 color;
  float padding0;
  mat4 cascade_view_projs[SHADOW_CASCADE_NUM];
  vec4 cascade_splits;
};

#endif // SHADERS_UBO_STRUCTURES_H