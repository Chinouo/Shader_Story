#version 460

layout(set = 0, binding = 0) uniform PerframeData {
  mat4 proj_view_matrix;
  vec3 camera_position;
}
perframe_data;

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec3 in_tangent;
layout(location = 3) in vec2 in_texcoord;

layout(location = 0) out vec4 out_position;
layout(location = 1) out vec2 out_texcoord;

void main() {
  gl_Position = perframe_data.proj_view_matrix * vec4(in_position, 1.0);
  out_texcoord = in_texcoord;
}