#ifndef GLOBAL_HPP
#define GLOBAL_HPP

#include <memory>

#include "engine/common/macros.h"

namespace ShaderStory {

class RenderSystem;
class WindowSystem;
class InputSystem;
class WorldManager;
class UIManager;
class RenderSwapData;

class RuntimeGlobalContext final {
 public:
  RuntimeGlobalContext() = default;
  ~RuntimeGlobalContext() = default;

  void StartSystems();
  void ShutDownSystems();

 public:
  std::shared_ptr<RenderSystem> m_render_sys;
  std::shared_ptr<WindowSystem> m_window_sys;
  std::shared_ptr<InputSystem> m_input_sys;
  std::shared_ptr<WorldManager> m_world_manager;
  std::shared_ptr<UIManager> m_ui_manager;

  std::unique_ptr<RenderSwapData> m_swap_context;

 private:
  DISALLOW_COPY_ASSIGN_AND_MOVE(RuntimeGlobalContext);
};

extern RuntimeGlobalContext g_runtime_global_context;

}  // namespace ShaderStory

#endif