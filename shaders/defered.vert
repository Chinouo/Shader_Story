#version 460

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

layout(location = 0) in vec3 in_position_ws;
layout(location = 1) in vec3 in_normal_ws;
layout(location = 2) in vec3 in_tangent_ws;
layout(location = 3) in vec2 in_texcoord;

layout(location = 0) out vec4 out_position_vs;
layout(location = 1) out vec3 out_normal_vs;
layout(location = 2) out vec2 out_texcoord;
layout(location = 3) out mat3 out_TBN;

void main() {
  vec4 vert_pos_vs = perframe_data.view_mat * vec4(in_position_ws, 1.0);
  gl_Position = perframe_data.proj_mat * vert_pos_vs;

  out_position_vs = vert_pos_vs;
  // cover normal in viewspace.
  mat4 normal_mat =
      perframe_data.view_mat * mat4(transpose(inverse(mat3(1.0))));

  // our mesh is static. all input is normalized.
  // construct TBN in view space.
  // TODO:
  mat3 view_rotate = mat3(perframe_data.view_mat);
  vec3 T = in_tangent_ws;
  vec3 N = in_normal_ws;
  vec3 B = cross(N, T);

  // ts -> ws -> vs.
  out_TBN = view_rotate * mat3(T, B, N);

  out_normal_vs = mat3(perframe_data.view_mat) * in_normal_ws;

  out_texcoord = in_texcoord;
}