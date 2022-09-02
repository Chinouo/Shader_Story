#ifndef GLOBAL_HPP
#define GLOBAL_HPP

#include <memory>

#include "engine/common/macros.h"

namespace ShaderStory {

class RenderSystem;
class WindowSystem;

class RuntimeGlobalContext final {
 public:
  RuntimeGlobalContext() = default;
  ~RuntimeGlobalContext() = default;

  void StartSystems();
  void ShutDownSystems();

 public:
  std::shared_ptr<RenderSystem> m_render_sys;
  std::shared_ptr<WindowSystem> m_window_sys;

 private:
  DISALLOW_COPY_ASSIGN_AND_MOVE(RuntimeGlobalContext);
};

extern RuntimeGlobalContext g_runtime_global_context;

}  // namespace ShaderStory

#endif