#ifndef LIGHTS_HPP
#define LIGHTS_HPP

#include <limits>

#include "engine/component/base_component.hpp"
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

class Sun : public DirectionLight, public ReflectUIComponent {
 public:
  Sun();
  ~Sun();

  void Tick(double delta_time) override;
  void OnDrawUI() const override;

  /// @deprecated
  mat4 GetViewProjMatrix(const RenderCamera& camera) const;

  mat4 GetViewProjMatrixSphereBounding(const RenderCamera& camera) const;

  /// @deprecated
  mat4 GetViewProjMatrixTest(const RenderCamera& camera) const;

  std::array<mat4, 3> GetCascadeViewProjMatrices(
      const RenderCamera& camera) const;

  void SetUpUIComponent();

 private:
  mat4 MakeCascadeViewProjMatrix(const mat4& view_proj_mat) const;

 private:
  mutable bool display_ui{true};
  // static const int cascade_count = 3;
  std::array<float, 4> m_cascade_distances{0.03f, 24.f, 80.f, 300.f};

};  // namespace ShaderStory

}  // namespace ShaderStory

#endif