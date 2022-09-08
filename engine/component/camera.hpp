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

  mat4 GetViewProjectionMatrix() const;

  vec3 GetPosition() const;

  void SetPosition(vec3&);
  void SetForward(vec3&);

  void ApplyStep(vec3&);
  void ApplyAngle(vec2&);

 protected:
  vec3 m_position{0.f, 0.f, 0.f};
  vec3 m_forward{0.f, 1.f, 0.f};
  vec3 m_up{0.f, 0.f, 1.f};
  vec3 m_right{1.f, 0.f, 0.f};

  float m_aspect{16.f / 9.f};
  float m_fov{45.f};
  float m_zfar{500.f};
  float m_znear{0.03f};

  // degress
  float m_yaw{0.f};
  float m_pitch{0.f};
};

/// @brief When process position, using Logic Tick,
/// but when process cursor, using listener and mutex for a smooth rotation.
class CameraComponent final : public RenderCamera {
 public:
  CameraComponent() = default;
  ~CameraComponent() = default;

  void Tick(double delta_time);

 private:
  float m_speed{0.3f};
  float m_cursor_sensitive{0.3};

  bool m_is_first_cursor_pos{true};
  float m_last_cursor_x{0.f};
  float m_last_cursor_y{0.f};

  // TODO: implement.
  std::mutex m_proj_view_mat_mtx;
};

}  // namespace ShaderStory

#endif