#ifndef SHADERS_CONSTANTS_H
#define SHADERS_CONSTANTS_H

/**
 * set-0 for engine-global resource
 * set-1 for material resource, for material only store this set
 * set-2 for per-object resource.
 */
#define GLOBAL_SET_INDEX 0
#define MATERIAL_SET_INDEX 1
#define PER_OBJECT_SET_INDEX 2
#define MAX_LIGHTS_COUNT 8
#define SHADOW_CASCADE_NUM 4

#endif // SHADERS_CONSTANTS_H