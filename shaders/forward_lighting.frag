#version 450
#extension GL_GOOGLE_include_directive : enable

#include "shader_structs.h"

layout(set = 1, binding = 0)  uniform _UMaterial {
	UMaterial material;
};

layout(location = 0) in vec2 uv;
layout(location=1) in vec3 normal;
layout(location=2) in vec3 pos;
layout(location = 0) out vec4 o_color;

void main()
{
	//MaterialInfo mat_info = calc_material_info();
	//o_color = calc_pbr(mat_info);
	//o_color = texture(material.base_color, uv);
	o_color = vec4(1.0, 0.0, 0.0, 1.0);
}