#version 450

layout(location=0) in vec2 uv;
layout(location=1) in vec3 normal;

// uniform binding = 2
layout(location=0) out vec4 frag_color; // layout location ==> attachment index, refer to glsl specification 4.4.2 output layout qualifiers

void main(void)
{
    frag_color = vec4(uv.x, uv.y, 1.0, 1.0);
}