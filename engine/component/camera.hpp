#ifndef CAMERA_HPP
#define CAMERA_HPP

#include <mutex>

#include "base_component.hpp"
#include "engine/common/macros.h"
#include "engine/core/math.hpp"

namespace ShaderStory {

/// @brief eular angle implemented camera.
class RenderCamera {
 public:
  RenderCamera() = default;
  ~RenderCamera() = default;

  mat4 GetViewMatrix() const;
  mat4 GetProjMatrix() const;

  mat4 GetViewProjectionMatrix() const;
  vec3 GetPosition() const;
  mat4 GetInverseProjectionViewMatrix() const;
  mat4 GetInverseProjectionViewMatrixCascadeUseOnly() const;

  mat4 DebugGetViewProjectionMatrix() const;

  mat4 GetInverseViewMatrix() const {
    return inverse(lookAt(m_position, m_position + m_forward, m_up));
  }

  float GetHalfFov() const { return m_fov / 2.f; };
  float GetFov() const { return m_fov; }
  float GetAspect() const { return m_aspect; };
  float GetZnear() const { return m_znear; }
  float GetZfar() const { return m_zfar; }

  void SetPosition(vec3&);
  void SetForward(vec3&);

  void ApplyStep(vec3&);
  void ApplyAngle(vec2&);

 protected:
  vec3 m_position{0.f, 0.f, 20.f};
  vec3 m_forward{world_front};
  vec3 m_up{world_up};
  vec3 m_right{world_right};

  float m_aspect{16.f / 9.f};
  float m_fov{45.f};
  float m_zfar{300.f};
  float m_znear{0.03f};

  // degress
  float m_yaw{0.f};
  float m_pitch{0.f};
};

/// @brief When process position, using Logic Tick,
/// but when process cursor, using listener and mutex for a smooth rotation.
class CameraComponent final : public RenderCamera, public ReflectUIComponent {
 public:
  CameraComponent() = default;
  ~CameraComponent() = default;

  void Tick(double delta_time);
  void OnDrawUI() override;
  void SetUpRenderRenderCameraUI();

 private:
  float m_speed{0.3f};
  float m_cursor_sensitive{0.3};

  bool m_is_first_cursor_pos{true};
  float m_last_cursor_x{0.f};
  float m_last_cursor_y{0.f};

  // ui
  mutable bool need_draw_ui{true};
  float last_input[3] = {0.0, 0.0, 0.0};

  // TODO: implement.
  std::mutex m_proj_view_mat_mtx;
};

}  // namespace ShaderStory

#endif
