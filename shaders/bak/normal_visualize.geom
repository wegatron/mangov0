#version 450
layout (triangles) in;
layout (line_strip, max_vertices = 6) out;

in VS_OUT {
  layout(location = 0) vec3 normal;
} gs_in[];

const float MAGNITUDE = 0.4;

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

void GenerateLine(int index)
{
    gl_Position = global_uniform.proj * gl_in[index].gl_Position;
    EmitVertex();
    gl_Position = global_uniform.proj * (gl_in[index].gl_Position + 
                                vec4(gs_in[index].normal, 0.0) * MAGNITUDE);
    EmitVertex();
    EndPrimitive();
}

void main()
{
    GenerateLine(0); // first vertex normal
    GenerateLine(1); // second vertex normal
    GenerateLine(2); // third vertex normal
} 