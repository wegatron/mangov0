#version 450
#extension GL_GOOGLE_include_directive : enable

#include "shader_structs.h"

// vertex data binding = 0
layout(location=0) in vec3 vpos;
layout(location=1) in vec3 normal;
layout(location=2) in vec2 uv;

layout(location=0) out vec2 out_uv;
layout(location=1) out vec3 out_normal; // world space normal
layout(location=2) out vec3 out_pos; // world space position

layout(push_constant) uniform _TransformPCO { TransformPCO transform_pco; };

void main()
{
    out_uv = uv;
    out_normal = normalize(mat3x3(transform_pco.nm) * normal);
    out_pos = (transform_pco.m * vec4(vpos, 1.0)).xyz;
    gl_Position = transform_pco.mvp * vec4(vpos, 1.0);
}
