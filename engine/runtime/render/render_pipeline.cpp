#include "engine/runtime/render/render_pipeline.hpp"

#include <iostream>

namespace ShaderStory {

RenderPipeline::RenderPipeline() {}

RenderPipeline::~RenderPipeline() {
  Dispose();
  std::cout << "Call `RenderPipeline Destory.`" << std::endl;
}

void RenderPipeline::SetRHI(std::shared_ptr<RHI::VKRHI> rhi) { m_rhi = rhi; }

void RenderPipeline::Initilaize() {
  test_pass = std::make_unique<TestPass>();
  ui_pass = std::make_unique<UIPass>();

  test_pass->PreInitialize(m_rhi);
  test_pass->Initialize();

  ui_pass->PreInitialize(m_rhi);
  ui_pass->SetVkPass(test_pass->GetVkRenderPassForUI());
  ui_pass->Initialize();
}

void RenderPipeline::Dispose() {
  m_rhi->WaitDeviceIdle();
  test_pass->Dispose();
  test_pass.reset();

  ui_pass->Dispose();
  ui_pass.reset();
}

void RenderPipeline::RecordCommands() {
  test_pass->RunPass();
  ui_pass->RunPass();
}

void RenderPipeline::RecreatePipeline() {
  Dispose();
  Initilaize();
}
}  // namespace ShaderStory