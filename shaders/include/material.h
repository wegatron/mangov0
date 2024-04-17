#ifdef SHADERS_MATERIAL_H
#define SHADERS_MATERIAL_H

#include "shader_structs.h"

struct MaterialInfo {
  vec3 position;
  vec3 normal;
  vec4 base_color;
  vec4 emissive_color;
  float metallic;
  float roughness;
  float occlusion;
};

MaterialInfo calc_material_info() {}

#endif // SHADERS_MATERIAL_H