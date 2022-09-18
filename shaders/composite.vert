#version 460

vec2 uv[6] = vec2[](vec2(-1.0, -1.0), vec2(-1.0, 1.0), vec2(1.0, -1.0),
                    vec2(-1.0, 1.0), vec2(1.0, -1.0), vec2(1.0, 1.0));

layout(location = 0) out vec2 out_uv;
layout(location = 1) out vec3 color;

void main() {
  if (gl_VertexIndex < 3) {
    color = vec3(1.0, 0.0, 0.0);
  } else {
    color = vec3(0.0, 0.0, 1.0);
  }

  gl_Position = vec4(uv[gl_VertexIndex], 0.0, 1.0);
  out_uv = uv[gl_VertexIndex] * 0.5 + 0.5;
}