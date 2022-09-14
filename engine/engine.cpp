#include "engine.hpp"

#include <chrono>
#include <iostream>

#include "engine/runtime/framework/world_manager.hpp"
#include "engine/runtime/global/global.hpp"
#include "engine/runtime/render/render_system.hpp"
#include "engine/runtime/window/window_system.hpp"

namespace ShaderStory {

void Engine::InitializeEngine() { g_runtime_global_context.StartSystems(); }

void Engine::RunInitialedEngine() {
  // 是否有代渲染的任务
  Semaphore render_data_ready(0);
  // logic 最多领先 n - 1 帧, 2类似 双重缓重， 3 类似 三重缓冲
  Semaphore race_frame_idx(k_swap_size);

  // render thread task.
  std::shared_ptr<RenderSystem> m_rd_sys =
      g_runtime_global_context.m_render_sys;

  auto render_task = [=, &render_data_ready, &race_frame_idx]() {
    int buf_idx = 0;

    long frame = 0;
    while (true) {
      render_data_ready.Wait();
      if (should_shutdown) break;

      auto render_task_start = std::chrono::steady_clock::now();
      TickRender(0.f);
      SwapRenderData(buf_idx);
      auto render_task_end = std::chrono::steady_clock::now();
      std::chrono::duration<double, std::milli> render_span =
          render_task_end - render_task_start;
      CalculateFPS(render_span.count());

      buf_idx = (buf_idx + 1) % k_swap_size;
      ++frame;
      race_frame_idx.Signal();
    }
    std::cout << "end render..\n";
  };
  std::thread render_thread(render_task);

  // logic thread task.
  auto logic_task = [this, &render_data_ready, &race_frame_idx]() {
    std::chrono::steady_clock clock;
    long frame = 0;
    int buf_idx = 0;
    while (true) {
      race_frame_idx.Wait();
      if (should_shutdown) break;

      auto logic_task_start = std::chrono::steady_clock::now();
      TickLogic(0.f);
      SwapLogicData(buf_idx);
      SwapData();
      auto logic_task_end = std::chrono::steady_clock::now();
      std::chrono::duration<double, std::milli> task_span =
          logic_task_end - logic_task_start;

      buf_idx = (buf_idx + 1) % k_swap_size;
      ++frame;
      render_data_ready.Signal();
    }

    std::cout << "logic end.\n";
  };
  std::thread logic_thread(logic_task);

  // main thread.
  std::shared_ptr<WindowSystem> m_wd_sys =
      g_runtime_global_context.m_window_sys;
  while (!m_wd_sys->ShouldCloseWindow()) {
    glfwWaitEvents();
  }

  should_shutdown = true;
  // To aviod dead lock, we need signal.
  render_data_ready.Signal();
  race_frame_idx.Signal();
  m_rd_sys->should_shutdown = true;
  DispatchShutDownMessage();
  render_thread.join();
  logic_thread.join();
}

void Engine::DebugSingleThread() {
  ASSERT(false);
  // std::shared_ptr<WindowSystem> m_wd_sys =
  //     g_runtime_global_context.m_window_sys;
  // while (!m_wd_sys->ShouldCloseWindow()) {
  //   glfwWaitEvents();
  //   TickLogic(1.f);
  //   g_runtime_global_context.m_render_sys->TickRender(1.f);
  // }
}

void Engine::ShutDowmEngine() { g_runtime_global_context.ShutDownSystems(); }

void Engine::TickRender(double delta_time) {
  g_runtime_global_context.m_render_sys->TickRender(delta_time);
}

void Engine::TickLogic(double delta_time) {
  g_runtime_global_context.m_world_manager->Tick(delta_time);
}

void Engine::SwapLogicData(int available_slot) {
  SwapData& cur_swap_data =
      g_runtime_global_context.m_swap_context->GetSwapData(available_slot);
  g_runtime_global_context.m_world_manager->LoadSwapData(cur_swap_data);
}

void Engine::SwapRenderData(int available_slot) {
  const SwapData& cur_swap_data =
      g_runtime_global_context.m_swap_context->GetSwapData(available_slot);
  g_runtime_global_context.m_render_sys->ConsumeSwapdata(cur_swap_data);
}

void Engine::DispatchShutDownMessage() {
  // m_rd_sys->should_shutdown = true;
}

void Engine::CalculateFPS(double delta_time) {
  // average sample, delta_time is millionsec
  ++m_frame_count;

  if (m_frame_count == 1) {
    m_average_duration = delta_time;
  } else {
    m_average_duration =
        m_average_duration * (1 - k_smooth_alpha) + delta_time * k_smooth_alpha;
  }

  m_fps = static_cast<int>(1000.f / m_average_duration);
}

}  // namespace ShaderStory