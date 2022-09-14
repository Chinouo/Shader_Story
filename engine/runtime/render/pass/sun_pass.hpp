#ifndef SUN_PASS_HPP
#define SUN_PASS_HPP

#include "engine/common/macros.h"
#include "engine/runtime/render/render_base.hpp"

namespace ShaderStory {

/// You can call it DirectionShadow Pass.
class SunPass : public RenderPassBase {
 public:
  SunPass();
  ~SunPass();

  void Initialze();
  void Dispose();

  void RunPass() override;

 private:
  void CreateVkRenderPass();
  void CreateDesciptorSetLayout();
  void CreateVkRenderPipeline();
  void CreateDesciptorSet();
  void CreateFrameBuffers();

  VkRenderPass m_sun_shadowmap_pass{VK_NULL_HANDLE};

  VkDescriptorSetLayout m_sun_shadowmap_desp_set_layout{VK_NULL_HANDLE};
  VkDescriptorSet m_shadowmap_desp_set{VK_NULL_HANDLE};

  VkPipelineLayout m_sun_shadowmap_pipeline_layout{VK_NULL_HANDLE};
  VkPipeline m_sun_shadowmap_pipeline{VK_NULL_HANDLE};
  std::vector<VkFramebuffer> m_framebuffers;
};

}  // namespace ShaderStory

#endif