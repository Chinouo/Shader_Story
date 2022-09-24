#version 460

layout(set = 0, binding = 0) uniform PerframeData {
  mat4 proj_view_matrix;
  mat4 camera_view_matrix;
  mat4 cascade_proj_view_matrices[3];
  float depth_splits[3];
  vec3 camera_position_ws;
  vec3 sun_ray_direction;
  vec3 sun_position_ws;
}
perframe_data;

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec3 in_tangent;
layout(location = 3) in vec2 in_texcoord;

layout(location = 0) out vec3 out_position_ws;
layout(location = 1) out vec3 out_normal_ws;
layout(location = 2) out vec2 out_texcoord;

void main() {
  vec4 clipSpacePosition =
      perframe_data.proj_view_matrix * vec4(in_position, 1.0);

  gl_Position = clipSpacePosition;
  out_position_ws = in_position;
  out_normal_ws = in_normal;
  out_texcoord = in_texcoord;
}