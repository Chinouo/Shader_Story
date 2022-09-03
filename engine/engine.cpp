#include "engine.hpp"

#include <chrono>
#include <iostream>

#include "engine/runtime/global/global.hpp"

namespace ShaderStory {

void Engine::InitializeEngine() { g_runtime_global_context.StartSystems(); }

void Engine::RunInitialedEngine() {
  // 是否有代渲染的任务
  Semaphore render_data_ready(0);
  // logic 最多领先几帧
  Semaphore race_frame_idx(1);

  // render thread task.
  std::shared_ptr<RenderSystem> m_rd_sys =
      g_runtime_global_context.m_render_sys;

  auto render_task = [=, &render_data_ready, &race_frame_idx]() {
    int buf_idx = 0;

    // std::chrono::steady_clock clock;
    // auto last_logic_frame_time = std::chrono::steady_clock::now();
    long frame = 0;
    while (true) {
      render_data_ready.Wait();
      if (should_shutdown) break;
      // std::cout << "tick render..\n";

      auto render_task_start = std::chrono::steady_clock::now();
      m_rd_sys->TickRender(0.f);
      auto render_task_end = std::chrono::steady_clock::now();
      std::chrono::duration<double, std::milli> render_span =
          render_task_end - render_task_start;

      CalculateFPS(render_span.count());
      // std::cout << "gpu time:" << render_span.count() << std::endl;
      //  std::cout << "r: " << buf_idx << std::endl;
      //   std::cout << "render thread time:" << render_span.count()
      //             << " f: " << frame << std::endl;
      buf_idx = (buf_idx + 1) % 2;
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
      // std::cout << "logic tick.\n";
      auto logic_task_start = std::chrono::steady_clock::now();
      TickLogic(0.f);
      auto logic_task_end = std::chrono::steady_clock::now();
      std::chrono::duration<double, std::milli> task_span =
          logic_task_end - logic_task_start;

      // std::cout << "cpu time:" << task_span.count() << std::endl;
      //  std::cout << "cpu time:" << task_span.count() << " f: " << frame
      //            << std::endl;
      //  std::cout << "l: " << buf_idx << std::endl;
      buf_idx = (buf_idx + 1) % 2;
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
  std::shared_ptr<WindowSystem> m_wd_sys =
      g_runtime_global_context.m_window_sys;
  while (!m_wd_sys->ShouldCloseWindow()) {
    glfwWaitEvents();
    TickLogic(1.f);
    g_runtime_global_context.m_render_sys->TickRender(1.f);
  }
}

void Engine::ShutDowmEngine() { g_runtime_global_context.ShutDownSystems(); }

void Engine::TickLogic(double delta_time) {
  // int val = arc4random() % 1000;
  // std::this_thread::sleep_for(std::chrono::milliseconds(5000));
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

  // std::cout << "FPS:" << m_fps << std::endl;
}

}  // namespace ShaderStory