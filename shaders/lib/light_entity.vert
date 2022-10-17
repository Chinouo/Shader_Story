#version 460

layout(set = 0, binding = 0) uniform PerframeUBO {
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
perframe_ubo;

struct PointLightObject {
  vec3 position;
  float padding_1;
  vec3 color;
  float padding_2;
  vec3 indensity;
  float padding_3;
};

struct DirectionObject {
  vec3 position;
  float padding_1;
  vec3 color;
  float padding_2;
};

layout(set = 0, binding = 1) uniform PerframeSBO {
  PointLightObject point_lights[5];
  DirectionObject direction_light[5];
}
perframe_sbo;

layout(location = 0) in vec3 in_position_ws;

void main() {
  vec3 translate = perframe_sbo.point_lights[gl_InstanceIndex].position;
  mat4 model = mat4(1.0);
  model[0][3] = translate.x;
  model[1][3] = translate.y;
  model[2][3] = translate.z;

  gl_Position = perframe_ubo.proj_view_mat * model * vec4(in_position_ws, 1.0);
}