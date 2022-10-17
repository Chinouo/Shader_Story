#ifndef ILLUMINATION_HPP
#define ILLUMINATION_HPP

#include "engine/component/lights.hpp"
#include "engine/runtime/render/render_swap_data.hpp"

namespace ShaderStory {

class Illumination final : public ReflectUIComponent {
 public:
  Illumination();
  ~Illumination();

  void OnDrawUI() override;

  void LoadSwapData(PerframeDataSBO& data);

 private:
  std::vector<PointLightComponent> m_point_lights;
  // std::vector<DirectionLightComponent> m_dir_lights;
  bool m_show_ui_component{true};
};

}  // namespace ShaderStory

#endif