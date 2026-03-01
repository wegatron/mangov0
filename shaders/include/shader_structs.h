#ifndef SHADERS_UBO_STRUCTURES_H
#define SHADERS_UBO_STRUCTURES_H

#include "shaders/include/constants.h"

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

enum LightType {
  LIGHT_DIRECTIONAL = 0,
  LIGHT_POINT = 1,
  //SPOT = 2,
  LIGHT_TYPE_NUM
};

struct ULighting {
  // lights
  UDirectionalLight directional_lights[MAX_DIRECTIONAL_LIGHT_NUM];
  UPointLight point_lights[MAX_POINT_LIGHT_NUM];

  ushort light_num[LIGHT_TYPE_NUM];
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