#version 450

// vertex data binding = 0
layout(location=0) in vec4 vpos;
layout(location=1) in vec3 normal;
layout(location=2) in vec2 uv;

// indices data binding = 1
layout(location=0) out vec2 o_uv;
layout(location=1) out vec3 o_normal;

layout(set=0, binding = 0) uniform GlobalUniform
{
    mat4 model;
    mat4 view_proj;
} global_uniform;

void main(void)
{
    gl_Position = global_uniform.view_proj * global_uniform.model * vpos;
    o_normal = mat3(global_uniform.model) * normal;    
    o_uv = uv;
}