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
  // test_pass->RunPass();
  sun_pass->RunPass();
  mesh_pass->RunPass();
  ui_pass->RunPass();
}

void RenderPipeline::RecreatePipeline() {
  DestoryPasses();
  SetupPasses();
}

void RenderPipeline::DestoryPasses() {
  sun_pass.reset();
  ui_pass.reset();
  mesh_pass.reset();
}

void RenderPipeline::SetupPasses() {
  sun_pass = std::make_unique<SunPass>();
  ui_pass = std::make_unique<UIPass>();
  mesh_pass = std::make_unique<MeshPass>();

  sun_pass->PreInitialize({m_rhi, m_resource});
  sun_pass->Initialze();

  mesh_pass->PreInitialize({m_rhi, m_resource});
  mesh_pass->Initialize();

  ui_pass->PreInitialize({m_rhi, m_resource});
  ui_pass->SetVkPass(mesh_pass->GetVkRenderPassForUI());
  ui_pass->Initialize();
}
}  // namespace ShaderStory
