#version 460

layout(set = 0, binding = 1) uniform sampler2D terrain_texture;

layout(location = 0) in vec3 in_position_ws;
layout(location = 1) in vec3 in_normal_ws;
layout(location = 2) in vec2 in_texcoord;

layout(location = 0) out vec4 out_position_ws;
layout(location = 1) out vec4 out_normal_ws;
layout(location = 2) out vec4 out_albedo;

void main() {
  out_position_ws = vec4(in_position_ws, 1.0);
  out_albedo = vec4(texture(terrain_texture, in_texcoord).rgb, 1.0);
  out_normal_ws = vec4(in_normal_ws, 1.0);
}