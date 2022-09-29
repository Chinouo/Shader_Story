#version 460 core

vec2 uv[6] = vec2[](vec2(-1.0, -1.0), vec2(-1.0, 1.0), vec2(1.0, -1.0),
                    vec2(-1.0, 1.0), vec2(1.0, -1.0), vec2(1.0, 1.0));

layout(location = 0) out vec2 out_uv;

void main() {
  gl_Position = vec4(uv[gl_VertexIndex], 0.0, 1.0);
  out_uv = uv[gl_VertexIndex] * 0.5 + 0.5;
}