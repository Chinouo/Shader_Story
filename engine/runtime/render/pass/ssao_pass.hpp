#ifndef SSAO_PASS_HPP
#define SSAO_PASS_HPP

#include <array>

#include "engine/runtime/render/render_base.hpp"

namespace ShaderStory {

class SSAOPass : public RenderPassBase {
 public:
  SSAOPass();
  ~SSAOPass();

  void RunPass() override;

  void Initialize();
  void Dispose();

 private:
  void CreateVkRenderPass();
  void CreateDesciptorSetLayout();
  void CreateVkRenderPipeline();
  void CreateDesciptorSet();
  void CreateFrameBuffers();

 private:
  VkPipelineLayout m_ssao_pip_layout{VK_NULL_HANDLE};
  VkPipeline m_ssao_pip{VK_NULL_HANDLE};
  VkRenderPass m_ssao_pass{VK_NULL_HANDLE};

  VkDescriptorSetLayout m_ssao_set_layout{VK_NULL_HANDLE};
  std::array<VkDescriptorSet, MAX_FRAMES_IN_FLIGHT> m_ssao_sets;

  std::array<VkFramebuffer, MAX_FRAMES_IN_FLIGHT> m_ssao_fbs;
};

}  // namespace ShaderStory

#endif