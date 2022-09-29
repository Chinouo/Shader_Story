#version 460 core

// #extension GL_GOOGLE_include_directive : enable

// #include "include/setting.h"

#define SSAO_SAMLE_RADIUS 0.5
#define SSAO_KERNAL_SIZE 32
#define SSAO_NOISE_DIM 4

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

layout(set = 0, binding = 1) uniform sampler2D GPositionTex;
layout(set = 0, binding = 2) uniform sampler2D GNormalTex;
layout(set = 0, binding = 3) readonly buffer SSAOKernal {
  vec4 kernals[SSAO_KERNAL_SIZE];
}
ssao_kernal;

layout(set = 0, binding = 4) uniform sampler2D SSAO_Noise;

layout(location = 0) in vec2 in_uv;

layout(location = 0) out float out_occlusion;

const vec2 noise_scale = vec2(800.0 / 4.0, 600.0 / 4.0);

void main() {
  vec3 frag_pos_vs = texture(GPositionTex, in_uv).xyz;
  vec3 rand_vec = normalize(texture(SSAO_Noise, in_uv * noise_scale).xyz);
  vec3 N = texture(GNormalTex, in_uv).xyz;
  vec3 T = vec3(0.0, 0.0, 1.0);
  vec3 B = cross(N, T);
  mat3 TBN = mat3(T, B, N);

  float occlusion = 0.0;

  for (uint i = 0; i < SSAO_KERNAL_SIZE; ++i) {
    // we sample in view space.
    vec3 sample_pos_vs = TBN * ssao_kernal.kernals[i].xyz;
    sample_pos_vs = frag_pos_vs.xyz + sample_pos_vs * SSAO_SAMLE_RADIUS;

    // proj sample
    vec4 offset = perframe_data.proj_mat * vec4(sample_pos_vs, 1.0);
    offset.xyz /= offset.w;
    offset.xyz = offset.xyz * 0.5 + 0.5;

    float sample_depth = texture(GPositionTex, offset.st).z;
    // range check

    occlusion += sample_depth > sample_pos_vs.z ? 1.0 : 0.0;
  }

  // ...
  out_occlusion = 1.0 - (occlusion / SSAO_KERNAL_SIZE);
}
