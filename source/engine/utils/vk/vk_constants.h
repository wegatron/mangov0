#pragma once

#include <shaders/include/constants.h>

namespace mango {
constexpr uint32_t VK_MAX_COMMAND_BUFFERS = 10;
constexpr uint32_t CONFIG_UNIFORM_BINDING_COUNT =
    10; // This is guaranteed by OpenGL ES.
constexpr uint32_t CONFIG_STORAGE_BINDING_COUNT =
    10; // This is guaranteed by OpenGL ES.
constexpr uint32_t CONFIG_SAMPLER_BINDING_COUNT =
    4; // This is guaranteed by OpenGL ES.
constexpr uint32_t DESCRIPTOR_TYPE_COUNT = 3;

constexpr uint32_t MAX_MAT_DESC_SET = 100;
constexpr uint32_t MAX_GLOBAL_DESC_SET = 100;
constexpr uint32_t MAX_TEXTURE_NUM_COUNT =
    4; // average max texture number for one descriptor set

constexpr uint32_t TIME_BEFORE_EVICTION = 4;
constexpr uint32_t DATA_RESOURCE_TIME_BEFORE_EVICTION = 7200;

constexpr uint32_t MAX_FRAMES_IN_FLIGHT = 3;
} // namespace mango