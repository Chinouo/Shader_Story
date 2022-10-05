#include "engine/runtime/framework/ui_manager.hpp"

#include "engine/runtime/global/global.hpp"
#include "engine/runtime/render/render_system.hpp"
#include "third_party/imgui/backends/imgui_impl_glfw.h"
namespace ShaderStory {

void UIManager::Initialize() {
  // vk backend setup is done by ui_pass.hpp
}

void UIManager::AddUIComponent(ReflectUIComponent* component) {
  m_registered_components.push_back(component);
}
void UIManager::RemoveComponent(ReflectUIComponent* component) {
  m_registered_components.push_back(component);
}

void UIManager::RecordUIComponentDrawCommand() {
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();

  for (auto* component : m_registered_components) {
    component->OnDrawUI();
  }
  // Insert your ui code here.
  bool t = true;
  ImGui::ShowDemoWindow(&t);

  ImGui::Begin("Recreate Test", &t, ImGuiWindowFlags_MenuBar);
  if (ImGui::Button("Recreate")) {
    g_runtime_global_context.m_render_sys->AddPostFrameCallback(
        []() { g_runtime_global_context.m_render_sys->ReloadPipeline(); });
  }
  if (ImGui::Button("Switch to GameMode")) {
    g_runtime_global_context.m_render_sys->AddPostFrameCallback([]() {
      g_runtime_global_context.m_window_sys->PerformModeChange(
          WindowMode::PLAY);
    });
  }

  ImGui::End();

  ImGui::Render();
}

}  // namespace ShaderStory