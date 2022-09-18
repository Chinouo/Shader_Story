#version 460

layout(set = 1, binding = 0) uniform sampler2D GPositionTex;
layout(set = 1, binding = 1) uniform sampler2D GNormalTex;
layout(set = 1, binding = 2) uniform sampler2D GAlbedoTex;

layout(location = 0) in vec2 in_uv;
layout(location = 1) in vec3 color;

layout(location = 0) out vec4 out_color;

void main() {
  out_color = texture(GAlbedoTex, in_uv).rgba;

  // out_color = vec4(color, 1.0);
}
