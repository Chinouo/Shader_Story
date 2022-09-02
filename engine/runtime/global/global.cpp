#include "engine/runtime/global/global.hpp"

#include "engine/runtime/render/render_system.hpp"
#include "engine/runtime/window/window_system.hpp"
namespace ShaderStory {

RuntimeGlobalContext g_runtime_global_context;

void RuntimeGlobalContext::StartSystems() {
  m_window_sys = std::make_shared<WindowSystem>(1280, 720);
  m_render_sys = std::make_shared<RenderSystem>();

  m_window_sys->Initialize();
  m_render_sys->Initialize();
}

void RuntimeGlobalContext::ShutDownSystems() {
  m_window_sys.reset();
  m_render_sys.reset();
}

}  // namespace ShaderStory