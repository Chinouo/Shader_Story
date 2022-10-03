#version 460

layout(set = 0, binding = 1) uniform sampler2D terrain_texture;
layout(set = 0, binding = 2) uniform sampler2D terrain_normalmap;
layout(set = 0, binding = 3) uniform sampler2D terrain_pbr_material;

layout(location = 0) in vec4 in_position_vs;
layout(location = 1) in vec3 in_normal_vs;
layout(location = 2) in vec2 in_texcoord;

layout(location = 0) out vec4 out_position_vs;
layout(location = 1) out vec4 out_normal_vs;
layout(location = 2) out vec4 out_albedo;
layout(location = 3) out vec4 out_pbr_material;
// vec4 out_depth depth write automatically.

void main() {
  out_position_vs = in_position_vs;
  out_albedo = vec4(texture(terrain_texture, in_texcoord).rgb, 1.0);
  out_normal_vs = vec4(in_normal_vs, 1.0);
  out_pbr_material = vec4(texture(terrain_pbr_material, in_texcoord).rgb, 1.0);
}