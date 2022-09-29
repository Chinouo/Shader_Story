#ifndef SSAO_RESOURCE_HPP
#define SSAO_RESOURCE_HPP

#include <array>

#include "engine/runtime/render/render_base.hpp"
namespace ShaderStory {

/// SSAO can integrate at composite pass, but we still want a split pass for
/// learning.

struct SSAOResource {
  VkImage ssao_image{VK_NULL_HANDLE};
  VkImageView ssao_image_view{VK_NULL_HANDLE};
  VmaAllocation ssao_alloc{VK_NULL_HANDLE};

  void Destory(VkDevice device, VmaAllocator allocator) {
    vmaDestroyImage(allocator, ssao_image, ssao_alloc);
    vkDestroyImageView(device, ssao_image_view, nullptr);
  }
};

class SSAOResourceManager : public RenderResourceBase {
 public:
  friend class RenderResource;

  SSAOResourceManager();
  ~SSAOResourceManager();

  std::array<VkWriteDescriptorSet, MAX_FRAMES_IN_FLIGHT> GetDespWrtiers() const;

  VkDescriptorBufferInfo GetSSAOKernalBufInfo() const;
  VkWriteDescriptorSet GetSSAOKernalWriteInfo() const;
  VkDescriptorImageInfo GetSSAONoiseImageInfo() const;
  VkWriteDescriptorSet GetSSAONoiseWriteInfo() const;

  const SSAOResource& GetSSAOResources(int idx) const {
    return m_ssao_res[idx];
  }

  VkDescriptorImageInfo GetSSAOTextureImageInfo(int idx) const;
  VkWriteDescriptorSet GetSSAOWTextureWrite() const;

 private:
  void Initialize(std::shared_ptr<RHI::VKRHI> rhi) override;
  void Destory(std::shared_ptr<RHI::VKRHI> rhi) override;

 private:
  std::array<SSAOResource, MAX_FRAMES_IN_FLIGHT> m_ssao_res;

  // noise texture
  VkImage m_ssao_noise{VK_NULL_HANDLE};
  VkImageView m_ssao_noise_view{VK_NULL_HANDLE};
  VmaAllocation m_ssao_noise_alloc{VK_NULL_HANDLE};

  // ssao kernal
  VkBuffer m_ssao_kernal_buf{VK_NULL_HANDLE};
  VmaAllocation m_ssao_kernal_alloc{VK_NULL_HANDLE};

  // noise sampler
  VkSampler m_ssao_noise_sampler{VK_NULL_HANDLE};
};

};  // namespace ShaderStory

#endif