// #ifndef MAIN_CAMERA_PASS
// #define MAIN_CAMERA_PASS
// #include <array>

// #include "engine/runtime/render/render_base.hpp"

// namespace ShaderStory {

// class MainCameraPass : public RenderPassBase {
//  public:
//   MainCameraPass();
//   ~MainCameraPass();

//   void Initialze();
//   void Dispose();

//   void RunPass() override;

//   VkRenderPass GetVkPass() const { return m_composite_pass; }

//  private:
//   void CreateVkRenderPass();
//   void CreateDesciptorSetLayout();
//   void CreateVkRenderPipeline();
//   void CreateDesciptorSet();
//   void CreateFrameBuffers();

//   // offscreen
//   VkRenderPass m_offscreen_pass{VK_NULL_HANDLE};
//   std::vector<VkFramebuffer> m_offscreen_framebuffers;
//   VkDescriptorSet m_offscreen_set{VK_NULL_HANDLE};
//   VkDescriptorSetLayout m_offscreen_set_layout{VK_NULL_HANDLE};
//   VkPipelineLayout m_offscreen_pipeline_layout{VK_NULL_HANDLE};
//   VkPipeline m_offscreen_pipeline{VK_NULL_HANDLE};

//   // composite
//   VkRenderPass m_composite_pass{VK_NULL_HANDLE};
//   // a set for dynamic buffer data.
//   VkDescriptorSet m_composite_dybuffer_set{VK_NULL_HANDLE};
//   VkDescriptorSetLayout m_composite_dybuffer_set_layout{VK_NULL_HANDLE};
//   // multi sets for g-buffer sample.
//   VkDescriptorSetLayout m_composite_gbuffer_set_layout{VK_NULL_HANDLE};
//   std::vector<VkDescriptorSet> m_composite_gbuffer_sets;
//   // multi sets for cascade-shadowmap
//   VkDescriptorSetLayout m_cascade_shadowmaps_set_layout{VK_NULL_HANDLE};
//   std::vector<VkDescriptorSet> m_cascade_shadowmap_sets;

//   VkPipelineLayout m_composite_pipeline_layout{VK_NULL_HANDLE};
//   VkPipeline m_composite_pipeline{VK_NULL_HANDLE};
//   std::vector<VkFramebuffer> m_composite_framebuffers;
// };

// }  // namespace ShaderStory

// #endif