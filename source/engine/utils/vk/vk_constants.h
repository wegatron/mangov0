#pragma once

namespace mango {
static constexpr uint32_t VK_MAX_COMMAND_BUFFERS = 10;
static constexpr uint32_t CONFIG_UNIFORM_BINDING_COUNT =
    10; // This is guaranteed by OpenGL ES.
static constexpr uint32_t CONFIG_STORAGE_BINDING_COUNT =
    10; // This is guaranteed by OpenGL ES.
static constexpr uint32_t CONFIG_SAMPLER_BINDING_COUNT =
    4; // This is guaranteed by OpenGL ES.
static constexpr uint32_t DESCRIPTOR_TYPE_COUNT = 3;

/**
 * set-0 for engine-global resource
 * set-1 for material resource, for material only store this set
 * set-2 for per-object resource.
 */
constexpr uint32_t GLOBAL_SET_INDEX = 0;
constexpr uint32_t MATERIAL_SET_INDEX = 1;
constexpr uint32_t PER_OBJECT_SET_INDEX = 2;

constexpr uint32_t MAX_MAT_DESC_SET = 100;
constexpr uint32_t MAX_GLOBAL_DESC_SET = 100;
constexpr uint32_t MAX_TEXTURE_NUM_COUNT =
    4; // average max texture number for one descriptor set
constexpr uint32_t MAX_LIGHTS_COUNT = 8;

static constexpr uint32_t TIME_BEFORE_EVICTION = 4;
static constexpr uint32_t DATA_RESOURCE_TIME_BEFORE_EVICTION = 7200;
} // namespace mango