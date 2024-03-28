#pragma once

#include <Eigen/Dense>
#include <engine/utils/vk/vk_constants.h>

namespace mango {

// for compatability with assimp
enum class LightType : uint32_t {  
  UNDEFINED = 0x0,
  DIRECTIONAL = 0x1,
  POINT = 0x2,
  SPOT = 0x3,
  AMBIENT = 0x4,
  AREA = 0x5
};

struct alignas(16) Light {
  LightType light_type;
  float inner_angle;
  float outer_angle;
  float falloff;

  //float preserved;
  alignas(16) Eigen::Vector4f position[4];
  alignas(16) Eigen::Vector3f direction;
  alignas(16) Eigen::Vector3f intensity;
};

struct Lights {  
  Light l[MAX_LIGHTS_COUNT];
  uint32_t lights_count{0};
  float reserve[3];
};
// filament/src/details/View.cpp --> prepare

} // namespace mango