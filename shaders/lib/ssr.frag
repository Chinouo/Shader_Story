#version 460 core

#define MAX_REFLECT_DISTANCE = 15.0;
#define RAY_MARCHING_MAX_STEPS 30
#define RAY_MARCHING_STEP_LENGTH 0.1

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

layout(binding = 0) uniform sampler2D GPosition;
layout(binding = 1) uniform sampler2D GAlbedo;
layout(binding = 2) uniform sampler2D GNormal;
layout(binding = 3) uniform sampler2D GDepth;

// Ray marching in view space.
// https://imanolfotia.com/blog/1

// Ray marching in clip space.
// https://lettier.github.io/3d-game-shaders-for-beginners/screen-space-reflection.html

vec4 SSR_RayMarching_ViewSpace(vec3 in_dir, const vec3 N, const vec3 view_pos) {
  in_dir = normalize(in_dir);
  vec3 reflected = reflect(in_dir, N);

  vec3 hit_pos_vs = view_pos;

  // Ray marching
  vec4 hit_coord;
  vec4 projcoord;
  vec3 dir = reflected * RAY_MARCHING_STEP_LENGTH;
  for (int i = 0; i < RAY_MARCHING_MAX_STEPS; ++i) {
    hit_pos_vs += dir;

    projcoord = perframe_data.proj_mat * vec4(hit_pos_vs, 1.0);
    projcoord.xy /= projcoord.w;
    projcoord.xy = projcoord.xy * 0.5 + 0.5;

    vec3 next_frag = texture(GPosition, projcoord.st).xyz;

    // ray inside object
    if (next_frag.z < hit_pos_vs.z) {
      return vec4(texture(GAlbedo, next_frag.st).rgb, 1.0);
    }
  }

  return vec4(1.0, 1.0, 1.0, 1.0);
}

vec4 SSR_RayMarching_ClipSpace(const vec3 N, const vec3 view_pos,
                               const vec3 hit_pos) {
  vec3 vi = normalize(view_pos - hit_pos);

  vec3 reflected = reflect(vi, N);

  // estimate marching distance
  vec3 start_uv;
  vec3 end_uv;

  int max_distance;
  for (int i = 0; i < max_distance; ++i) {
    vec4 projcoord;
  }
  return vec4(0.0);
}