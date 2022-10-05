#include "engine/component/camera.hpp"

#include <iostream>

#include "engine/runtime/framework/ui_manager.hpp"

namespace ShaderStory {

mat4 RenderCamera::GetViewProjectionMatrix() const {
  mat4 proj = perspective(m_fov, m_aspect, m_znear, m_zfar);
  // proj[0][0] *= -1;
  proj[1][1] *= -1;
  mat4 view = lookAt(m_position, m_position + m_forward, m_up);
  return proj * view;
};

mat4 RenderCamera::GetInverseProjectionViewMatrix() const {
  return inverse(GetViewProjectionMatrix());
};

mat4 RenderCamera::GetInverseProjectionViewMatrixCascadeUseOnly() const {
  mat4 proj = perspective(m_fov, m_aspect, m_znear, m_zfar);
  proj[1][1] *= -1;
  mat4 view = lookAt(m_position, m_position + m_forward, m_up);
  return inverse(proj * view);
};

mat4 RenderCamera::GetViewMatrix() const {
  return lookAt(m_position, m_position + m_forward, m_up);
}

mat4 RenderCamera::GetProjMatrix() const {
  mat proj = perspective(m_fov, m_aspect, m_znear, m_zfar);
  proj[1][1] *= -1;
  return proj;
}

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
  m_forward.y = sin(rp);
  m_forward.z = sin(ry) * cos(rp);

  m_right = cross(world_up, m_forward);
  m_up = cross(m_forward, m_right);

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

void CameraComponent::SetUpRenderRenderCameraUI() {
  g_runtime_global_context.m_ui_manager->AddUIComponent(this);
}

void CameraComponent::OnDrawUI() {
  ImGui::Begin("RenderCamera", &need_draw_ui, ImGuiWindowFlags_MenuBar);
  ImGui::Text("Position: x: %.3f y: %.3f z: %.3f", m_position.x, m_position.y,
              m_position.z);
  ImGui::Text("Foward: x: %.3f y: %.3f z: %.3f", m_forward.x, m_forward.y,
              m_forward.z);
  ImGui::Text("Right: x: %.3f y: %.3f z: %.3f", m_right.x, m_right.y,
              m_right.z);
  ImGui::Text("Up: x: %.3f y: %.3f z: %.3f", m_up.x, m_up.y, m_up.z);
  ImGui::InputFloat3("c:", last_input);
  if (ImGui::Button("Set CameraPosition")) {
    auto tsf = vec3(last_input[0], last_input[1], last_input[2]);
    SetPosition(tsf);
  }

  ImGui::End();
};

}  // namespace ShaderStory
