#ifndef ENGINE_HPP
#define ENGINE_HPP

#include <memory>
#include <thread>

#include "engine/common/events.hpp"
#include "engine/common/macros.h"
#include "engine/core/semaphore.hpp"
#include "engine/runtime/io/input_system.hpp"

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
  void TickRender(double delta_time);

  void SwapLogicData(int available_slot);
  void SwapRenderData(int available_slot);

  void DispatchShutDownMessage();

  void CalculateFPS(double delta_time);

 private:
  bool should_shutdown{false};

  const double k_smooth_alpha{1.0 / 100.0};
  u_int64_t m_frame_count{0};
  double m_average_duration{0.0};
  int m_fps{0};

  // std::shared_ptr<InputSystem> m_input_sys;

  const int k_swap_size = 2;

  DISALLOW_COPY_ASSIGN_AND_MOVE(Engine);
};

}  // namespace ShaderStory

#endif