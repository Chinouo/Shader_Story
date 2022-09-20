#include "engine/runtime/global/global.hpp"

#include "engine/runtime/framework/ui_manager.hpp"
#include "engine/runtime/framework/world_manager.hpp"
#include "engine/runtime/io/input_system.hpp"
#include "engine/runtime/render/render_system.hpp"
#include "engine/runtime/window/window_system.hpp"

namespace ShaderStory {

RuntimeGlobalContext g_runtime_global_context;

void RuntimeGlobalContext::StartSystems() {
  m_window_sys = std::make_shared<WindowSystem>(1280, 720);
  m_render_sys = std::make_shared<RenderSystem>();
  m_input_sys = std::make_shared<InputSystem>();
  m_world_manager = std::make_shared<WorldManager>();
  m_ui_manager = std::make_shared<UIManager>();

  m_window_sys->Initialize();
  m_render_sys->Initialize();
  m_input_sys->Initialize();
  m_world_manager->Initialize();
  m_ui_manager->Initialize();

  m_swap_context = std::make_unique<RenderSwapData>();
}

void RuntimeGlobalContext::ShutDownSystems() {
  m_window_sys.reset();
  m_render_sys.reset();
  m_input_sys.reset();
}

}  // namespace ShaderStory