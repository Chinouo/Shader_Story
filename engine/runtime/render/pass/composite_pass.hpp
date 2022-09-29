#ifndef COMPOSITE_PASS_HPP
#define COMPOSITE_PASS_HPP

#include <array>

#include "engine/runtime/render/render_base.hpp"
namespace ShaderStory {

class CompositePass : public RenderPassBase {
 public:
  CompositePass();
  ~CompositePass();
  void RunPass() override;
  void Initialize();
  void Dispose();

  VkRenderPass GetVkPass() const { return m_composite_pass; }

 private:
  void CreateVkRenderPass();
  void CreateDesciptorSetLayout();
  void CreateVkRenderPipeline();
  void CreateDesciptorSet();
  void CreateFrameBuffers();

 private:
  // composite
  VkRenderPass m_composite_pass{VK_NULL_HANDLE};
  // a set for dynamic buffer data.
  VkDescriptorSet m_composite_dybuffer_set{VK_NULL_HANDLE};
  VkDescriptorSetLayout m_composite_dybuffer_set_layout{VK_NULL_HANDLE};
  // multi sets for g-buffer sample.
  VkDescriptorSetLayout m_composite_gbuffer_set_layout{VK_NULL_HANDLE};
  std::vector<VkDescriptorSet> m_composite_gbuffer_sets;
  // multi sets for cascade-shadowmap
  VkDescriptorSetLayout m_cascade_shadowmaps_set_layout{VK_NULL_HANDLE};
  std::vector<VkDescriptorSet> m_cascade_shadowmap_sets;

  // multi sets for SSAO texture sampling.
  VkDescriptorSetLayout m_ssao_set_layout{VK_NULL_HANDLE};
  std::array<VkDescriptorSet, MAX_FRAMES_IN_FLIGHT> m_ssao_sets{VK_NULL_HANDLE};

  VkPipelineLayout m_composite_pipeline_layout{VK_NULL_HANDLE};
  VkPipeline m_composite_pipeline{VK_NULL_HANDLE};
  std::vector<VkFramebuffer> m_composite_framebuffers;
};
}  // namespace ShaderStory

#endif