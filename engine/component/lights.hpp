#ifndef LIGHTS_HPP
#define LIGHTS_HPP

#include <limits>

#include "engine/component/base_component.hpp"
#include "engine/component/camera.hpp"
#include "engine/core/math.hpp"
namespace ShaderStory {

typedef struct {
  mat4 cascade_proj_view_matrix;
  float split_depth;
} CascadeData;

class DirectionLight {
 public:
  virtual ~DirectionLight() = default;
  virtual void Tick(double delta_time) = 0;

  void SetPosition(const vec3& pos) { m_position = pos; };
  void SetDirection(const vec3& dir) { m_direction = dir; };

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
  // void Tick(double delta_time);

 protected:
  vec3 m_position;
  vec3 m_emission;
};

class PointLightComponent : public PointLight, public ReflectUIComponent {
 public:
  PointLightComponent() = default;
  ~PointLightComponent() = default;

  void SetPosition(const vec3& pos) { m_position = pos; };

  virtual void OnDrawUI() {
    ImGui::Begin("PointLight", &display_ui, ImGuiWindowFlags_MenuBar);

    ImGui::Text("PointLight Position: x: %.3f y: %.3f z: %.3f", m_position.x,
                m_position.y, m_position.z);

    ImGui::InputFloat3("PointLight Input:", last_pos_input);
    if (ImGui::Button("Set PointLight Pos")) {
      auto tsf = vec3(last_pos_input[0], last_pos_input[1], last_pos_input[2]);
      SetPosition(tsf);
    }

    ImGui::End();
  }

 private:
  bool display_ui{true};
  float last_pos_input[3] = {0.f, 0.f, 0.f};
};

class Sun : public DirectionLight, public ReflectUIComponent {
 public:
  Sun();
  ~Sun();

  void Tick(double delta_time) override;
  void OnDrawUI() override;

  /// @deprecated
  mat4 GetViewProjMatrix(const RenderCamera& camera) const;

  mat4 GetViewProjMatrixSphereBounding(const RenderCamera& camera) const;

  /// @deprecated
  mat4 GetViewProjMatrixTest(const RenderCamera& camera) const;

  std::array<CascadeData, 3> GetCascadeViewProjMatrices(
      const RenderCamera& camera) const;

  void SetUpUIComponent();

 private:
  mat4 MakeCascadeViewProjMatrix(const mat4& view_proj_mat) const;

 private:
  // ui code
  mutable bool display_ui{true};
  float last_pos_input[3] = {0.f, 0.f, 0.f};
  float last_dir_input[3] = {0.f, 0.f, 0.f};

  // static const int cascade_count = 3;
  //  std::array<float, 4> m_cascade_distances{0.03f, 24.f, 80.f, 300.f};

};  // namespace ShaderStory

}  // namespace ShaderStory

#endif
