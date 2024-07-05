#ifndef SHADERS_UBO_STRUCTURES_H
#define SHADERS_UBO_STRUCTURES_H

#include "shaders/include/constants.h"

#ifdef __cplusplus
#include <Eigen/Dense>

using vec3 = Eigen::Vector3f;
using vec4 = Eigen::Vector4f;
using mat4 = Eigen::Matrix4f;
using mat3 = Eigen::Matrix3f;
using uint = uint32_t;
#endif // __cplusplus

struct UMaterial {
  uint albedo_type;
  uint emissive_type;
  uint metallic_roughness_occlution_type;
  uint padding0;
  vec4 albedo_color;
  vec4 emissive_color;
  vec4 metallic_roughness_occlution;
};

struct UDirectionalLight {
  vec4 direction; //!< direction, last float for padding
  vec4 illuminance; //!< lux(lm/m^2), last float for padding
};

struct UPointLight {
  vec4 position; //!< position, last float for padding
  vec4 luminous_intensity; //!< candela(cd), last float for padding
};

struct ULighting {
  // lights
  UDirectionalLight directional_light[MAX_DIRECTIONAL_LIGHT_NUM];
  UPointLight point_lights[MAX_POINT_LIGHT_NUM];

  int directional_light_num;
  int point_light_num;
  float ev100;
  // // debug
  // vec3 camera_dir;
  // int shader_debug_option;
};

// struct ShadowCascadeUBO {
//   mat4 cascade_view_projs[SHADOW_CASCADE_NUM];
// };

// struct ShadowCubeUBO {
//   mat4 face_view_projs[SHADOW_FACE_NUM];
// };

struct TransformPCO {
  mat4 m;  // model matrix
  mat4 nm; // normal matrix, model matrix may have non-uniform scaling, so nm =
           // transpose(inverse(m))
  mat4 mvp;
};

#endif // SHADERS_UBO_STRUCTURES_H