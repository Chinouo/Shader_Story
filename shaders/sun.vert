#version 460

layout(set = 0, binding = 0) uniform PerframeData {
  mat4 proj_view_matrix;
  mat4 sun_proj_view_matrix;
  vec3 camera_position;
}
perframe_data;

layout(location = 0) in vec3 position;

void main() {
  gl_Position = perframe_data.sun_proj_view_matrix * vec4(position, 1.0);
}