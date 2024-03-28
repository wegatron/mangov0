#version 450
#extension GL_EXT_scalar_block_layout : require

struct LightT
{
  int light_type;
  float inner_angle;
  float outer_angle;
  float falloff;

  vec3 position[4]; // position[1..3] for area light

  vec3 direction;
  vec3 intensity; // lux for directional light or cd for other lights
};

layout(std430, set=GLOBAL_SET_INDEX, binding = 0) uniform GlobalUniform
{
    // camera
    vec3 camera_pos;
    float ev; // 16, [0.65 * 2^(ev100)]
    mat4 view; // 80
    mat4 proj; // 144

    // lights
    LightT lights[MAX_LIGHTS_COUNT]; // 144 + 64 * MAX_LIGHTS_COUNT
    int light_count;  // 144 + 64 * MAX_LIGHTS_COUNT + 16
} global_uniform;

// AREA LIGHT LTC Lookup table
layout(set=GLOBAL_SET_INDEX, binding=1) uniform sampler2D LTC1;
layout(set=GLOBAL_SET_INDEX, binding=2) uniform sampler2D LTC2;

const float LUT_SIZE  = 64.0; // ltc_texture size
const float LUT_SCALE = (LUT_SIZE - 1.0)/LUT_SIZE;
const float LUT_BIAS  = 0.5/LUT_SIZE;

layout(std430, set=MATERIAL_SET_INDEX, binding = 0) uniform BasicMaterial
{
    vec4 base_color;
    float metallic;
    float roughness;
    float specular;
}pbr_mat;

#ifdef HAS_BASE_COLOR_TEXTURE
layout(set=MATERIAL_SET_INDEX, binding = 1) uniform sampler2D base_color_tex;
#endif

#ifdef HAS_METALLIC_TEXTURE
layout(set=MATERIAL_SET_INDEX, binding = 2) uniform sampler2D metallic_tex;
#endif

#ifdef HAS_ROUGHNESS_TEXTURE
layout(set=MATERIAL_SET_INDEX, binding = 3) uniform sampler2D roughness_tex;
#endif

#ifdef HAS_METALLIC_ROUGHNESS_TEXTURE
layout(set=MATERIAL_SET_INDEX, binding = 4) uniform sampler2D metallic_roughness_tex;
#endif

#ifdef HAS_SPECULAR_TEXTURE
layout(set=MATERIAL_SET_INDEX, binding = 5) uniform sampler2D specular_tex;
#endif

#ifdef HAS_NORMAL_MAP
layout(set=MATERIAL_SET_INDEX, binding = 6) uniform sampler2D normal_map;
#endif

layout(location=0) in vec2 uv;
layout(location=1) in vec3 normal;
layout(location=2) in vec3 pos;
layout(location=0) out vec4 frag_color; // layout location ==> attachment index, refer to glsl specification 4.4.2 output layout qualifiers

const float PI = 3.14159265359f;

struct PixelShadingParam
{
  float NdotL;
  float NdotV;
  float LdotH;
  float NdotH;

  // material
  vec4 base_color;
  float metallic;
  float roughness;
  float specular;

  // light
  vec3 illumance;
};

vec3 F_Schlick (const vec3 f0 , const vec3 f90 , float u)
{
  return f0 + (f90 - f0) * pow (1.0f - u, 5.f);
}

float Fr_DisneyDiffuse(float NdotV, float NdotL, float LdotH, float roughness)
{
  // do energy conservation
  float energyBias = 0.5f * roughness;
  float energyFactor = mix(1.0f, 1.0f / 1.51f, roughness);
  vec3 fd90 = vec3(energyBias + 2.0f * LdotH*LdotH*roughness);
  vec3 f0 = vec3(1.0f);
  float lightScatter = F_Schlick(f0, fd90, NdotL).r;
  float viewScatter = F_Schlick(f0, fd90, NdotV).r;
  return lightScatter * viewScatter * energyFactor;
}

float D_GGX(const float NdotH, const float roughness)
{
  float sq_r = roughness * roughness;
  float f = NdotH*NdotH*(sq_r - 1.0f) + 1.0f;
  return sq_r / (PI * f * f);
}

float V_SmithGGXCorrelated(const float NdotL, const float NdotV, const float roughness)
{
  float sq_r = roughness * roughness;
  float lambda_GGXV = NdotL * sqrt(NdotV * NdotV *(1-sq_r) + sq_r);
  float lambda_GGXL = NdotV * sqrt(NdotL * NdotL *(1-sq_r) + sq_r);
  return 0.5f/max(1e-4f, (lambda_GGXL + lambda_GGXV));
}

vec3 surfaceShading(const PixelShadingParam pixel)
{
  // diffuse
  vec3 diffuse_color = pixel.base_color.rgb * (1.0f - pixel.metallic);
  float Fd = Fr_DisneyDiffuse(pixel.NdotV, pixel.NdotL, pixel.LdotH, pixel.roughness); // diffuse
  
  // reflection: D, F, G
  vec3 f0 = mix(vec3(0.16f * pixel.specular * pixel.specular), pixel.base_color.rgb, pixel.metallic);
  vec3 f90 = clamp(50.0f * f0, 0.0f, 1.0f);
  vec3 F = F_Schlick(f0, f90, pixel.LdotH);
  float D = D_GGX(pixel.NdotH, pixel.roughness);
  float Vis = V_SmithGGXCorrelated(pixel.NdotL, pixel.NdotV, pixel.roughness);
  vec3 Fr = D * Vis * F;
  
  return pixel.illumance * ((1.0f - F) * Fd * diffuse_color + Fr);
}

// Vector form without project to the plane (dot with the normal)
// Use for proxy sphere clipping
vec3 IntegrateEdgeVec(vec3 v1, vec3 v2)
{
    // Using built-in acos() function will result flaws
    // Using fitting result for calculating acos()
    float x = dot(v1, v2);
    float y = abs(x);

    float a = 0.8543985 + (0.4965155 + 0.0145206*y)*y;
    float b = 3.4175940 + (4.1616724 + y)*y;
    float v = a / b;

    float theta_sintheta = (x > 0.0) ? v : 0.5*inversesqrt(max(1.0 - x*x, 1e-7)) - v;

    return cross(v1, v2)*theta_sintheta;
}

vec3 LTC_Evaluate(vec3 N, vec3 V, vec3 P, mat3 Minv, int index)
{
  vec3 points[4] = global_uniform.lights[index].position;
  // construct orthonormal basis around N
  vec3 T1, T2;
  T1 = normalize(V - N * dot(V, N));
  T2 = cross(N, T1);

  // rotate area light in (T1, T2, N) basis
  Minv = Minv * transpose(mat3(T1, T2, N));

  // polygon (allocate 4 vertices for clipping)
  vec3 L[4];
  // transform polygon from LTC back to origin Do (cosine weighted)
  L[0] = Minv * (points[0] - P);
  L[1] = Minv * (points[1] - P);
  L[2] = Minv * (points[2] - P);
  L[3] = Minv * (points[3] - P);

  // use tabulated horizon-clipped sphere
  // check if the shading point is behind the light
  vec3 dir = points[0] - P; // LTC space
  vec3 lightNormal = cross(points[1] - points[0], points[3] - points[0]);
  bool behind = (dot(dir, lightNormal) < 0.0);

  // cos weighted space
  L[0] = normalize(L[0]);
  L[1] = normalize(L[1]);
  L[2] = normalize(L[2]);
  L[3] = normalize(L[3]);

  // integrate
  vec3 vsum = vec3(0.0);
  vsum += IntegrateEdgeVec(L[0], L[1]);
  vsum += IntegrateEdgeVec(L[1], L[2]);
  vsum += IntegrateEdgeVec(L[2], L[3]);
  vsum += IntegrateEdgeVec(L[3], L[0]);

  // form factor of the polygon in direction vsum
  float len = length(vsum);

  float z = vsum.z/len;
  if (behind)
      z = -z;

  vec2 uv = vec2(z*0.5f + 0.5f, len); // range [0, 1]
  uv = uv*LUT_SCALE + LUT_BIAS;

  // Fetch the form factor for horizon clipping
  float scale = texture(LTC2, uv).w;

  float sum = len*scale;

  // Outgoing radiance (solid angle) for the entire polygon
  vec3 Lo_i = vec3(sum, sum, sum);
  return Lo_i;
}

// vec3 ltcShading(const PixelShadingParam pixel, const vec3 N, const vec3 V, const vec3 P, const int light_index)
// {
//   // use roughness and sqrt(1-cos_theta) to sample M_texture
//   vec2 uv = vec2(pixel.roughness, sqrt(1.0f - pixel.NdotV));
//   uv = uv*LUT_SCALE + LUT_BIAS;

//   // get 4 parameters for inverse_M
//   vec4 t1 = texture(LTC1, uv);
//   vec4 t2 = texture(LTC2, uv);
  
//   mat3 Minv = mat3(
//       vec3(t1.x, 0, t1.y),
//       vec3(  0,  1,    0),
//       vec3(t1.z, 0, t1.w)
//   );

//   vec3 f0 = mix(vec3(0.16f * pixel.specular * pixel.specular), pixel.base_color.rgb, pixel.metallic);
//   // GGX BRDF shadowing and Fresnel
//   // t2.x: shadowedF90 (F90 normally it should be 1.0)
//   // t2.y: Smith function for Geometric Attenuation Term, it is dot(V or L, H).
//   vec3 F = f0 * t2.x + (1.0f - f0) * t2.y; // shadowing and Fres
//   vec3 Fd = LTC_Evaluate(N, V, P, mat3(1), light_index);
//   vec3 Fr = F * LTC_Evaluate(N, V, P, Minv, light_index);
//   vec3 diffuse_color = pixel.base_color.rgb * (1.0f - pixel.metallic);
//   return pixel.illumance * ((1.0f - F) * Fd * diffuse_color + Fr);
// }

vec3 ltcShading(const PixelShadingParam pixel, const vec3 N, const vec3 V, const vec3 P, const int light_index)
{
  // use roughness and sqrt(1-cos_theta) to sample M_texture
  vec2 uv = vec2(pixel.roughness, sqrt(1.0f - pixel.NdotV));
  uv = uv*LUT_SCALE + LUT_BIAS;

  // get 4 parameters for inverse_M
  vec4 t1 = texture(LTC1, uv);
  vec4 t2 = texture(LTC2, uv);
  
  mat3 Minv = mat3(
      vec3(t1.x, 0, t1.y),
      vec3(  0,  1,    0),
      vec3(t1.z, 0, t1.w)
  );

  // vec3 f0 = mix(vec3(0.16f * pixel.specular * pixel.specular), pixel.base_color.rgb, pixel.metallic);
  // // GGX BRDF shadowing and Fresnel
  // // t2.x: shadowedF90 (F90 normally it should be 1.0)
  // // t2.y: Smith function for Geometric Attenuation Term, it is dot(V or L, H).
  // vec3 F = f0 * t2.x + (1.0f - f0) * t2.y; // shadowing and Fres
  // vec3 Fd = LTC_Evaluate(N, V, P, mat3(1), light_index);
  // vec3 Fr = F * LTC_Evaluate(N, V, P, Minv, light_index);
  // vec3 diffuse_color = pixel.base_color.rgb * (1.0f - pixel.metallic);
  return LTC_Evaluate(N, V, P, Minv, light_index);
}

void main(void)
{
  PixelShadingParam pixel;
  
  #ifdef HAS_BASE_COLOR_TEXTURE
  pixel.base_color = texture(base_color_tex, uv);
  #else
  pixel.base_color = pbr_mat.base_color;
  #endif

  pixel.metallic = pbr_mat.metallic;
  pixel.roughness = pbr_mat.roughness;
  #ifdef HAS_METALLIC_ROUGHNESS_TEXTURE
  vec2 mr = texture(metallic_roughness_tex, uv).rg;
  pixel.metallic = mr[0];
  pixel.roughness = mr[1];
  #endif
  #ifdef HAS_METALLIC_TEXTURE
  pixel.metallic = texture(metallic_tex, uv).r;
  #endif
  #ifdef HAS_ROUGHNESS_TEXTURE
  pixel.roughness = texture(roughness_tex, uv).r;
  #endif

  #ifdef HAS_SPECULAR_TEXTURE
  pixel.specular = texture(specular_tex, uv).r;
  #else
  pixel.specular = pbr_mat.specular;
  #endif

  vec3 out_illumance = vec3(0.0f);
  //vec3 V = normalize(global_uniform.camera_pos - pos);
  vec3 V = normalize(vec3(0, 1, 0.5) - pos);
  pixel.NdotV = max(0.0f, dot(normal, V));
  for(int index=0; index<global_uniform.light_count; ++index)
  {
    if(global_uniform.lights[index].light_type == DIRECTIONAL)
    {
      pixel.NdotL = max(0.0f, dot(normal, -global_uniform.lights[index].direction));
      vec3 H = normalize(-global_uniform.lights[index].direction + V);  
      pixel.NdotH = max(0.0f, dot(normal, H));
      pixel.LdotH = max(0.0f, dot(-global_uniform.lights[index].direction, H));
      pixel.illumance = pixel.NdotL * global_uniform.lights[index].intensity;
      out_illumance += surfaceShading(pixel);
    }
    else if(global_uniform.lights[index].light_type == AREA)
    {
      pixel.illumance = global_uniform.lights[index].intensity;
      //out_illumance = LTC_Evaluate(normal, V, pos, mat3(1.0), 0);
      out_illumance += ltcShading(pixel, normal, V, pos, index);
    }
  }
  frag_color = vec4(out_illumance, 1.0f);
  //frag_color = vec4(global_uniform.ev * out_illumance, 1.0f);
}