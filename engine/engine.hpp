#ifndef ENGINE_HPP
#define ENGINE_HPP

#include <memory>
#include <thread>

#include "engine/common/events.hpp"
#include "engine/common/macros.h"
#include "engine/core/semaphore.hpp"
#include "engine/runtime/io/input_system.hpp"
#include "engine/runtime/render/render_system.hpp"
#include "engine/runtime/window/window_system.hpp"
namespace ShaderStory {

struct LogicRenderSyncObject {
  LogicRenderSyncObject();
  Semaphore s1;
  Semaphore s2;
};

/// global context.
class Engine {
 public:
  Engine() = default;
  ~Engine() = default;

  void InitializeEngine();

  void RunInitialedEngine();

  void ShutDowmEngine();

  void DebugSingleThread();

 private:
  void TickLogic(double delta_time);

  void DispatchShutDownMessage();

 private:
  bool should_shutdown{false};

  // std::shared_ptr<InputSystem> m_input_sys;

  DISALLOW_COPY_ASSIGN_AND_MOVE(Engine);
};

}  // namespace ShaderStory

#endif