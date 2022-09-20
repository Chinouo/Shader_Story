#version 460

layout(set = 0, binding = 0) uniform PerframeData {
  mat4 proj_view_matrix;
  mat4 camera_view_matrix;
  mat4 cascade_proj_view_matrices[3];
  vec3 camera_position;
  vec3 sun_ray_direction;
  vec3 sun_position_ws;
}
perframe_data;

layout(set = 1, binding = 0) uniform sampler2D GPositionTex;
layout(set = 1, binding = 1) uniform sampler2D GNormalTex;
layout(set = 1, binding = 2) uniform sampler2D GAlbedoTex;
// camera projected depth, need divide w.
layout(set = 1, binding = 3) uniform sampler2D GDepth;

layout(set = 2, binding = 0) uniform sampler2DArray cascade_shadowmaps;

layout(location = 0) in vec2 in_uv;
layout(location = 1) in vec3 color;

layout(location = 0) out vec4 out_color;

void main() {
  vec3 frag_position_ws = texture(GPositionTex, in_uv).xyz;

  float z = (perframe_data.camera_view_matrix * vec4(frag_position_ws, 1.0)).z;

  // direction light
  // vec4 frag_position_sun_space =
  //     perframe_data.cascade_proj_view_matrices[0] *
  //     vec4(frag_position_ws, 1.0);
  // vec2 shdow_uv = frag_position_sun_space.st * 0.5 + 0.5;

  // float frag_depth = frag_position_sun_space.z;
  // float sdmap_depth = texture(cascade_shadowmaps, vec3(shdow_uv, 0.0)).r;
  // if (frag_depth < sdmap_depth) {
  //   out_color = vec4(0.0, 0.0, 1.0, 1.0);
  // } else {
  //   out_color = texture(GAlbedoTex, in_uv).rgba;
  // }

  if (z > 0.0 && z < 24.0) {
    out_color = vec4(1.0, 0.0, 0.0, 1.0);
  } else if (z >= 24.0 && z < 80.0) {
    out_color = vec4(1.0, 1.0, 0.0, 1.0);
  } else if (z >= 80.0 && z <= 300.0) {
    out_color = vec4(1.0, 1.0, 1.0, 1.0);
  }

  // out_color = texture(GNormalTex, in_uv).rgba;

  // float d = texture(cascade_shadowmaps, vec3(shdow_uv, 3.0)).r;
  // out_color = vec4(vec3(d), 1.0);

  // out_color = vec4(color, 1.0);
}
