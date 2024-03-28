#version 450

// vertex data binding = 0
layout(location=0) in vec4 vpos;
layout(location=1) in vec3 normal;
layout(location=2) in vec2 uv;

layout(location=0) out vec2 out_uv;

layout(set=GLOBAL_SET_INDEX, binding = 0) uniform GlobalUniform
{
    // camera
    mat4 view;
    mat4 proj;
} global_uniform;

layout(set=PER_OBJECT_SET_INDEX, binding = 0) uniform MeshUniform
{
    mat4 model;
} mesh_uniform;

void main(void)
{
    gl_Position = global_uniform.proj * global_uniform.view * mesh_uniform.model * vpos;
    out_uv = uv;
}