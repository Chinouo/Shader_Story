#version 460

layout(set = 0, binding = 0) uniform PerframeData {
  mat4 proj_view_matrix;
  mat4 sun_proj_view_matrix;
  vec3 camera_position;
  vec3 sun_ray_direction;
  vec3 sun_position_ws;
}
perframe_data;

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec3 in_tangent;
layout(location = 3) in vec2 in_texcoord;

layout(location = 0) out vec3 out_frag_position_ws;
layout(location = 1) out vec3 out_frag_normal;
layout(location = 2) out vec2 out_frag_texcoord;
layout(location = 3) out vec4 out_frag_sun_space;

void main() {
  out_frag_position_ws = in_position;

  out_frag_texcoord = in_texcoord;
  out_frag_normal = in_normal;

  gl_Position = perframe_data.proj_view_matrix * vec4(in_position, 1.0);

  out_frag_sun_space =
      perframe_data.sun_proj_view_matrix * vec4(in_position, 1.0);
}