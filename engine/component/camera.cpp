#include "engine/component/camera.hpp"

#include <iostream>

namespace ShaderStory {

mat4 RenderCamera::GetViewProjectionMatrix() const {
  mat4 proj = perspective(m_fov, m_aspect, m_znear, m_zfar);
  proj[1][1] *= -1;
  mat4 view = lookAt(m_position, m_position + m_forward, m_up);
  return proj * view;
};

mat4 RenderCamera::DebugGetViewProjectionMatrix() const {
  mat4 proj = ortho(-200.f, 200.f, -200.f, 200.f, 0.03f, 500.f);
  proj[1][1] *= -1;
  mat4 view = lookAt(m_position, m_position + m_forward, m_up);
  return proj * view;
}

vec3 RenderCamera::GetPosition() const { return m_position; }

void RenderCamera::SetPosition(vec3& position) { m_position = position; }

void RenderCamera::SetForward(vec3& forward) {
  // m_position + forward
  // m_forward = forward;
}

void RenderCamera::ApplyAngle(vec2& delta) {
  // delta is dx, dy of cursor movement delta.
  m_pitch = clamp(m_pitch + delta.y, -89.f, 89.f);
  m_yaw += delta.x;
  auto rp = radians(m_pitch);
  auto ry = radians(m_yaw);

  m_forward.x = -cos(ry) * cos(rp);
  m_forward.y = sin(ry) * cos(rp);
  m_forward.z = sin(rp);

  m_right = cross(m_forward, world_up);
  m_up = cross(m_right, m_forward);

  m_forward = normalize(m_forward);
  m_right = normalize(m_right);
  m_up = normalize(m_up);
}

void RenderCamera::ApplyStep(vec3& delta) { m_position += delta; }

void CameraComponent::Tick(double delta_time) {
  u_int32_t command =
      g_runtime_global_context.m_input_sys->GetCurrentGamenCommandState();

  vec3 delta(0.f, 0.f, 0.f);
  if (command & (u_int32_t)GameCommand::forward) {
    delta += m_speed * m_forward;
  }
  if (command & (u_int32_t)GameCommand::backward) {
    delta -= m_speed * m_forward;
  }
  if (command & (u_int32_t)GameCommand::right) {
    delta += m_speed * m_right;
  }
  if (command & (u_int32_t)GameCommand::left) {
    delta -= m_speed * m_right;
  }
  ApplyStep(delta);

  auto cursor_state = g_runtime_global_context.m_input_sys->GetCursorPosState();

  // TODO: fix first cursor dash bug.
  if (!m_is_first_cursor_pos) {
    float delta_x = cursor_state.x - m_last_cursor_x;
    float delta_y = cursor_state.y - m_last_cursor_y;
    if (delta_x != 0.0 && delta_y != 0.0) {
      // reverse y.
      vec2 angle_delta = vec2(delta_x, -delta_y) * m_cursor_sensitive;
      ApplyAngle(angle_delta);
    }
  }
  m_last_cursor_x = cursor_state.x;
  m_last_cursor_y = cursor_state.y;
  m_is_first_cursor_pos = false;
}

}  // namespace ShaderStory
