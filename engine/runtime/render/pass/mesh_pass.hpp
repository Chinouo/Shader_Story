#ifndef MESH_PASS_HPP
#define MESH_PASS_HPP

#include "engine/runtime/render/render_base.hpp"

namespace ShaderStory {

struct MeshPassDepthObject {
  VkImage depth_image{VK_NULL_HANDLE};
  VkImageView depth_image_view{VK_NULL_HANDLE};
  VmaAllocation depth_alloc{VK_NULL_HANDLE};

  void Dispose(VmaAllocator allocator, VkDevice device) {
    vkDestroyImageView(device, depth_image_view, nullptr);
    vmaDestroyImage(allocator, depth_image, depth_alloc);
  }
};

class MeshPass final : public RenderPassBase {
 public:
  MeshPass();
  ~MeshPass();

  void Initialize();
  void RunPass() override;

  VkRenderPass GetVkRenderPassForUI() const { return m_mesh_pass; }

 private:
  void CreateDepthResources();
  void CreateVkRenderPass();
  void CreateDesciptorSetLayout();
  void CreateVkRenderPipeline();
  void CreateDesciptorSet();
  void CreateFrameBuffers();

  VkFormat PickDepthFormat();

  void Dispose();

  VkRenderPass m_mesh_pass{VK_NULL_HANDLE};

  VkDescriptorSetLayout m_mesh_desp_set_layout{VK_NULL_HANDLE};
  VkDescriptorSet m_mesh_desp_set{VK_NULL_HANDLE};
  /// check:
  /// https://stackoverflow.com/questions/36772607/vulkan-texture-rendering-on-multiple-meshes/36781650#36781650
  VkDescriptorSet m_mesh_texture_set{VK_NULL_HANDLE};

  VkPipelineLayout m_mesh_pipeline_layout{VK_NULL_HANDLE};
  VkPipeline m_mesh_pipeline{VK_NULL_HANDLE};
  std::vector<VkFramebuffer> m_framebuffers;
  std::vector<MeshPassDepthObject> m_depth_objects;

  DISALLOW_COPY_ASSIGN_AND_MOVE(MeshPass);
};

}  // namespace ShaderStory

#endif