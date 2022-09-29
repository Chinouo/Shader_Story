#include "engine/runtime/render/render_pipeline.hpp"

#include <iostream>

#include "engine/runtime/render/render_base.hpp"

namespace ShaderStory {

RenderPipeline::RenderPipeline() {}

RenderPipeline::~RenderPipeline() {
  std::cout << "Call `RenderPipeline Destory.`" << std::endl;
}

void RenderPipeline::Initilaize(
    std::shared_ptr<RHI::VKRHI> rhi,
    std::shared_ptr<RenderResource> render_resource) {
  m_rhi = rhi;
  m_resource = render_resource;

  SetupPasses();
}

void RenderPipeline::RecordCommands() {
  sun_pass->RunPass();
  defered_pass->RunPass();
  ssao_pass->RunPass();
  composite_pass->RunPass();

  ui_pass->RunPass();
}

void RenderPipeline::RecreatePipeline() {
  DestoryPasses();
  SetupPasses();
}

void RenderPipeline::DestoryPasses() {
  ssao_pass->Dispose();
  ssao_pass.reset();
  defered_pass->Dispose();
  defered_pass.reset();
  composite_pass->Dispose();
  composite_pass.reset();
  sun_pass->Dispose();
  sun_pass.reset();
  ui_pass.reset();
}

void RenderPipeline::SetupPasses() {
  defered_pass = std::make_unique<DeferedPass>();
  defered_pass->PreInitialize({m_rhi, m_resource});
  defered_pass->Initialize();

  ssao_pass = std::make_unique<SSAOPass>();
  ssao_pass->PreInitialize({m_rhi, m_resource});
  ssao_pass->Initialize();

  composite_pass = std::make_unique<CompositePass>();
  composite_pass->PreInitialize({m_rhi, m_resource});
  composite_pass->Initialize();

  sun_pass = std::make_unique<SunPass>();
  ui_pass = std::make_unique<UIPass>();

  sun_pass->PreInitialize({m_rhi, m_resource});
  sun_pass->Initialze();

  ui_pass->PreInitialize({m_rhi, m_resource});
  ui_pass->SetVkPass(composite_pass->GetVkPass());
  ui_pass->Initialize();
}
}  // namespace ShaderStory
