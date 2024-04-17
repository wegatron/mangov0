#version 450
//#extension GL_EXT_scalar_block_layout : require

// vertex data binding = 0
layout(location=0) in vec3 vpos;
layout(location=1) in vec3 normal;

layout(location=0) out vec3 out_normal;

struct LightT
{
  int light_type;
  float angle;
  float blend;
  float falloff;//16

  vec3 position;
  vec3 direction;
  vec3 color; // 64
  float intensity; // 64
};

layout(std430, set=GLOBAL_SET_INDEX, binding = 0) uniform GlobalUniform
{
    // camera
    float ev100; // 16
    mat4 view; // 80
    mat4 proj; // 144    

    // lights
    LightT lights[MAX_LIGHTS_COUNT]; // 144 + 64 * MAX_LIGHTS_COUNT
    int light_count;  // 144 + 64 * MAX_LIGHTS_COUNT + 16
} global_uniform;

layout(set=PER_OBJECT_SET_INDEX, binding = 0) uniform MeshUniform
{
    mat4 model;
} mesh_uniform;

void main(void)
{
    out_normal = mat3x3(global_uniform.view * mesh_uniform.model) * normal;
    gl_Position = global_uniform.view * mesh_uniform.model * vec4(vpos, 1.0f);
}