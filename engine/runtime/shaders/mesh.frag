#version 460

layout(set = 0, binding = 1) uniform sampler2D terrain_texture;

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec2 in_texcoord;

layout(location = 0) out vec4 color;

void main() { color = vec4(texture(terrain_texture, in_texcoord).rgb, 1.0); }