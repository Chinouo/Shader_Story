#version 460

layout(set = 0, binding = 0) uniform sampler2D light_texture;

layout(location = 0) in vec4 in_position_vs;
layout(location = 1) in vec3 in_normal_vs;
layout(location = 2) in vec2 in_texcoord;
layout(location = 3) in mat3 in_TBN;  // covert normal to view space.

layout(location = 0) out vec4 out_position_vs;
layout(location = 1) out vec4 out_normal_vs;
layout(location = 2) out vec4 out_albedo;
layout(location = 3) out vec4 out_pbr_material;
// vec4 out_depth depth write automatically.

void main() {
  out_normal_vs = vec4(0.0);
  out_position_vs = vec4(0.0);
  out_pbr_material = vec4(0.0);
  // we only need albedo.
  out_albedo = vec4(texture(light_texture, in_texcoord).rgb, 1.0);
}