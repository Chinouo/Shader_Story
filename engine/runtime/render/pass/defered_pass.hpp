#ifndef DEFERED_PASS_HPP
#define DEFERED_PASS_HPP

#include <array>

#include "engine/runtime/render/render_base.hpp"
namespace ShaderStory {

class DeferedPass : public RenderPassBase {
 public:
  DeferedPass();
  ~DeferedPass();
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
  // draw light entity.
  void CreateLightEntityPipeline();
  void CreateLightEntitySetLayout();
  void CreateLightDescriptorSet();

 private:
  VkRenderPass m_offscreen_pass{VK_NULL_HANDLE};
  std::array<VkFramebuffer, MAX_FRAMES_IN_FLIGHT> m_offscreen_framebuffers;
  VkDescriptorSet m_offscreen_set{VK_NULL_HANDLE};
  VkDescriptorSetLayout m_offscreen_set_layout{VK_NULL_HANDLE};
  VkPipelineLayout m_offscreen_pipeline_layout{VK_NULL_HANDLE};
  VkPipeline m_offscreen_pipeline{VK_NULL_HANDLE};
};

}  // namespace ShaderStory

#endif