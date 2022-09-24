#version 460

layout(push_constant) uniform CascadeBlock { uint cascade_matrix_index; }
cascade_block;

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

layout(location = 0) in vec3 position;

void main() {
  gl_Position =
      perframe_data
          .cascade_proj_view_matrices[cascade_block.cascade_matrix_index] *
      vec4(position, 1.0);
}