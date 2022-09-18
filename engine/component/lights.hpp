#ifndef LIGHTS_HPP
#define LIGHTS_HPP

#include <limits>

#include "engine/component/camera.hpp"
#include "engine/core/math.hpp"

namespace ShaderStory {

class DirectionLight {
 public:
  virtual ~DirectionLight() = default;
  virtual void Tick(double delta_time) = 0;

  vec3 GetDirection() const { return m_direction; }
  vec3 GetPosition() const { return m_position; }

 protected:
  vec3 m_position;
  vec3 m_emission;
  vec3 m_direction;
};

class PointLight {
 public:
  PointLight() = default;
  ~PointLight() = default;
  void Tick(double delta_time);

 protected:
  vec3 m_position;
  vec3 m_emission;
  vec3 m_direction;
};

class Sun : public DirectionLight {
 public:
  Sun();
  ~Sun();

  void Tick(double delta_time) override;

  /// @deprecated
  mat4 GetViewProjMatrix(const RenderCamera& camera) const;

  mat4 GetViewProjMatrixSphereBounding(const RenderCamera& camera) const;

  /// @deprecated
  mat4 GetViewProjMatrixTest(const RenderCamera& camera) const;

 private:
  static const int cascade_count = 3;

  float m_cascade_plane[cascade_count];

};  // namespace ShaderStory

}  // namespace ShaderStory

#endif