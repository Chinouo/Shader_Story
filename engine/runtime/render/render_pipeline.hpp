#ifndef RENDER_PIPELINE_HPP
#define RENDER_PIPELINE_HPP
#include <memory>

#include "engine/common/macros.h"
#include "engine/runtime/render/pass/test_pass.hpp"
#include "engine/runtime/render/pass/ui_pass.hpp"
#include "engine/runtime/render/rhi/render_base.hpp"

namespace ShaderStory {
class RenderPipeline final : public RenderPipelineBase {
 public:
  RenderPipeline();
  ~RenderPipeline();

  void SetRHI(std::shared_ptr<RHI::VKRHI> rhi);

  void Initilaize() override;
  void Dispose() override;
  void RecreatePipeline();

  void RecordCommands();

 private:
  DISALLOW_COPY_ASSIGN_AND_MOVE(RenderPipeline);

  std::unique_ptr<UIPass> ui_pass;
  std::unique_ptr<TestPass> test_pass;

  std::shared_ptr<RHI::VKRHI> m_rhi;
};

};  // namespace ShaderStory

#endif