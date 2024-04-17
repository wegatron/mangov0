#version 450
#extension GL_GOOGLE_include_directive : enable

//#include "pbr.h"

layout(location = 0) out vec4 o_color;

void main()
{
	//MaterialInfo mat_info = calc_material_info();
	//o_color = calc_pbr(mat_info);
	o_color = vec4(1.0, 0.0, 0.0, 1.0);
}