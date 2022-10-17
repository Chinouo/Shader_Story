#version 460 core

#extension GL_GOOGLE_include_directive : enable

#include "include/common.h"
#include "include/common_data.glsl"
#include "include/surface_shading.glsl"

layout(set = 0, binding = 0) uniform PerframeData {
  mat4 proj_view_mat;
  mat4 proj_mat;
  mat4 view_mat;
  mat4 cascade_proj_view_mats[3];
  vec4 depth_splits;
  vec3 camera_pos_ws;
  // float padding_1;
  vec3 sun_ray_dir;
  // float padding_2;
  vec3 sun_pos_ws;
  // float padding_3;
}
perframe_data;

layout(set = 0, binding = 1) readonly buffer PerframeSBOData {
  PointLight point_lights[MAX_POINT_LIGHT_COUNT];
  DirectionalLight direction_lights[MAX_DIRECTION_LIGHT_COUNT];
  uint point_light_count;
  uint direction_light_count;
}
perframe_sbo_data;

// all in view space
layout(set = 1, binding = 0) uniform sampler2D GPositionTex;
layout(set = 1, binding = 1) uniform sampler2D GNormalTex;
layout(set = 1, binding = 2) uniform sampler2D GAlbedoTex;
// camera projected depth, need divide w.
layout(set = 1, binding = 3) uniform sampler2D GDepth;
// RGBA:R: roughness G: metallic B: emissive
layout(set = 1, binding = 4) uniform sampler2D GPBRMaterial;

layout(set = 2, binding = 0) uniform sampler2DArray cascade_shadowmaps;

// sampler ssao factor,  frag which have great posibility for occlusion has a
// grearter val.
layout(set = 3, binding = 0) uniform sampler2D SSAOTex;

layout(location = 0) in vec2 in_uv;

layout(location = 0) out vec4 out_color;

vec3 Bling_Phong(const vec3 V, const vec3 N, const vec3 L, const vec3 albedo) {
  vec3 H = normalize(V + L);
  float ambient = 0.2;

  float diffuse = max(dot(N, L), 0.0);
  float specular = pow(max(dot(H, N), 0.0), 32.0);

  return (ambient + diffuse + specular) * albedo;
}

void main() {
  vec3 frag_position_vs = texture(GPositionTex, in_uv).xyz;
  vec3 frag_albedo = texture(GAlbedoTex, in_uv).rgb;
  vec3 frag_normal_vs = texture(GNormalTex, in_uv).xyz;  // already normalized

  float roughness = texture(GPBRMaterial, in_uv).r;
  float metallic = texture(GPBRMaterial, in_uv).g;
  float emissive = texture(GPBRMaterial, in_uv).b;

  float occlusion = texture(SSAOTex, in_uv).r;

  vec3 sun_pos_vs =
      (perframe_data.view_mat * vec4(perframe_data.sun_pos_ws, 1.0)).xyz;

  // vec3 L = normalize(sun_pos_vs - frag_position_vs);
  vec3 V = normalize(-frag_position_vs);
  vec3 N = frag_normal_vs;

  vec3 Lo = vec3(0.0);

  // bug: mineways export texture texcoord error!
  vec3 Le = frag_albedo * emissive * 10.0;

  // point light contribute.
  for (uint i = 0; i < perframe_sbo_data.point_light_count; ++i) {
    vec3 light_pos_ws = perframe_sbo_data.point_lights[i].position.xyz;
    vec3 light_pos_vs = (perframe_data.view_mat * vec4(light_pos_ws, 1.0)).xyz;

    vec3 L = -normalize(light_pos_vs - frag_position_vs);

    // float radius_atteuation = 1.0 / perframe_sbo_data.point_lights[i].radius;
    float distance_attenuation = length(light_pos_vs - frag_position_vs);
    distance_attenuation *= distance_attenuation;
    distance_attenuation = 1.0 / distance_attenuation;

    float attenuation =
        distance_attenuation * perframe_sbo_data.point_lights[i].intensity;

    // hack ambient
    vec3 Fa = vec3(0.02) * frag_albedo;

    Lo += surface_shading(V, L, N, frag_albedo, roughness, metallic) *
              attenuation +
          Fa;
  }

  // direction light contribute

  // transparent object not implement current.
  out_color = vec4(Lo, 1.0);

  out_color = vec4(frag_normal_vs, 1.0);
}
