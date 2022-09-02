
#include "engine/runtime/render/render_system.hpp"

#include <chrono>
#include <memory>

#include "engine/runtime/global/global.hpp"
#include "engine/runtime/window/window_system.hpp"

namespace ShaderStory {

RenderSystem::RenderSystem() {}

RenderSystem::~RenderSystem() {}

void RenderSystem::Initialize() {
  m_rhi = std::make_shared<RHI::VKRHI>();
  m_pipeline = std::make_unique<RenderPipeline>();

  m_rhi->Initialize(g_runtime_global_context.m_window_sys->GetWindow());

  m_pipeline->SetRHI(m_rhi);
  m_pipeline->Initilaize();
}

void RenderSystem::ReloadPipeline() { m_pipeline->RecreatePipeline(); }

void RenderSystem::Destory() {
  // m_pipeline->Dispose();
  // m_rhi->Destory();
  // m_pipeline.reset();

  // auto sz = m_rhi.use_count();
  // ASSERT(m_rhi.unique());
}

void RenderSystem::AddPostFrameCallback(std::function<void()>&& closure) {
  m_post_frame_callbacks.emplace_back(closure);
}

void RenderSystem::TickRender(double delta_time) {
  // int val = arc4random() % 1000;
  // std::this_thread::sleep_for(std::chrono::milliseconds(val));

  // prepare render data

  // wait fence sync
  m_rhi->WaitForFence();
  m_rhi->ResetCommandPool();

  // begin frame
  m_rhi->WarmUpBeforePass();

  // render one frame
  m_pipeline->RecordCommands();

  // end frame
  m_rhi->SubmitRenderingTask();

  // flush post frame call back
  for (const auto& callback : m_post_frame_callbacks) {
    callback();
  }
  m_post_frame_callbacks.clear();
}

}  // namespace ShaderStory