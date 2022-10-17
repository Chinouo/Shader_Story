#include "engine/runtime/framework/illumination.hpp"

#include "engine/runtime/framework/ui_manager.hpp"
#include "engine/runtime/global/global.hpp"

namespace ShaderStory {

Illumination::Illumination() {
  m_point_lights.resize(3);
  g_runtime_global_context.m_ui_manager->AddUIComponent(this);
}

Illumination::~Illumination() {}

void Illumination::OnDrawUI() {
  // ImGui::ShowDemoWindow()
  ImGui::Begin("Illumination Manager", &m_show_ui_component,
               ImGuiWindowFlags_MenuBar);

  for (int i = 0; i < m_point_lights.size(); ++i) {
    m_point_lights[i].OnDrawUI();
  }

  ImGui::End();
}

void Illumination::LoadSwapData(PerframeDataSBO& data) {
  data.current_point_light_count = m_point_lights.size();
  assert(m_point_lights.size() <= MAX_POINT_LIGHT_SIZE);

  for (size_t i = 0; i < m_point_lights.size(); ++i) {
    data.point_lights[i] = m_point_lights[i].GetPerframeData();
  }
}

}  // namespace ShaderStory