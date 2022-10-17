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

typedef struct {
  vec3 position;
  float radius;
  vec3 color;
  float intensity;
} PointLightSwapData;

typedef struct {
  vec3 position;
  float padding_1;
  vec3 color;
  float padding_2;

} DirectionLightSwapData;

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

 protected:
  vec3 m_position{0.f, 0.f, 0.f};
  float m_radius{1.f};
  vec3 m_color{1.f, 1.f, 1.f};
  float m_intensity{1.f};
};

class PointLightComponent : public PointLight {
 public:
  PointLightComponent()
      : id(reinterpret_cast<uint64_t>(this)){

        };
  ~PointLightComponent() = default;

  void SetPosition(const vec3& pos) { m_position = pos; };
  void SetIntensity(const float intensity) { m_intensity = intensity; }
  void SetColor(const vec3& color) { m_color = color; }

  void OnDrawUI() {
    // ImGui::Begin("PointLight", &display_ui, ImGuiWindowFlags_MenuBar);
    const std::string lable = std::to_string(id);
    const std::string unique_lable = "##" + lable;

    if (ImGui::CollapsingHeader(lable.c_str())) {
      ImGui::Text("PointLight Position: x: %.3f y: %.3f z: %.3f", m_position.x,
                  m_position.y, m_position.z);

      if (ImGui::InputFloat3((pos_lable + unique_lable).c_str(),
                             last_pos_input)) {
        const vec3 next_pos =
            vec3(last_pos_input[0], last_pos_input[1], last_pos_input[2]);
        SetPosition(next_pos);
      }

      if (ImGui::InputFloat3((color_lable + unique_lable).c_str(),
                             last_color_input)) {
        const vec3 next_color =
            vec3(last_color_input[0], last_color_input[1], last_color_input[2]);
        SetColor(next_color);
      }

      if (ImGui::InputFloat((indensity_lable + unique_lable).c_str(),
                            &last_indensity_input)) {
        const float next_intensity = last_indensity_input;
        SetIntensity(next_intensity);
      }
    }
  }

  PointLightSwapData GetPerframeData() const {
    return {
        m_position,
        m_radius,
        m_color,
        m_intensity,
    };
  }

 private:
  bool display_ui{true};

  const uint64_t id;

 private:
  float last_pos_input[3] = {m_position.x, m_position.y, m_position.z};
  float last_color_input[3] = {m_color.x, m_color.y, m_color.z};
  float last_indensity_input = {m_intensity};

  inline static const std::string pos_lable = "Position: ";
  inline static const std::string color_lable = "Color: ";
  inline static const std::string indensity_lable = "Indensity: ";
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
