#version 450
#extension GL_GOOGLE_include_directive : enable

#include "shader_structs.h"

layout(set=0, binding = 0) uniform _ULighting {
	ULighting lighting;
};

layout(set = 1, binding = 0)  uniform _UMaterial {
	UMaterial material;
};

layout(set = 1, binding = 1) uniform sampler2D albedo_map;
layout(set = 1, binding = 2) uniform sampler2D normal_map;
layout(set = 1, binding = 3) uniform sampler2D emissive_map;
layout(set = 1, binding = 4) uniform sampler2D metallic_roughness_occlusion_map;

layout(location = 0) in vec2 uv;
layout(location=1) in vec3 normal;
layout(location=2) in vec3 pos;
layout(location = 0) out vec4 o_color;

void main()
{
	//MaterialInfo mat_info = calc_material_info();
	//o_color = calc_pbr(mat_info);
	o_color = texture(albedo_map, uv);
	//o_color = vec4(1.0, 0.0, 0.0, 1.0);
}