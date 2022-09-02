#pragma once

#include <vulkan/vulkan.hpp>

#include "engine/runtime/render/rhi/render_base.hpp"
#include "engine/runtime/render/vk_macros.h"

namespace ShaderStory {

class TestPass final : public RenderPassBase {
 public:
  TestPass() = default;
  ~TestPass() = default;

  void Initialize();
  void RunPass() override;
  void Dispose() override;

  VkRenderPass GetVkRenderPassForUI() const { return m_test_pass; }

 private:
  void CreateVkRenderPass();
  void CreateVkRenderPipeline();
  void CreateFrameBuffers();

  VkRenderPass m_test_pass;
  VkPipelineLayout m_test_pipeline_layout;
  VkPipeline m_test_pipeline;
  std::vector<VkFramebuffer> m_framebuffers;
};

};  // namespace ShaderStory