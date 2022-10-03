#ifndef DEFERED_RESOURCE_HPP
#define DEFERED_RESOURCE_HPP

#include <array>

#include "engine/runtime/render/render_base.hpp"

namespace ShaderStory {

struct GBufferObject {
  // Define: Position in View Space(sampler uv in clip space).
  VkImage gPositionImg{VK_NULL_HANDLE};
  VkImageView gPositionView{VK_NULL_HANDLE};
  VmaAllocation gPositionAlloc{VK_NULL_HANDLE};

  VkImage gColorImg{VK_NULL_HANDLE};
  VkImageView gColorView{VK_NULL_HANDLE};
  VmaAllocation gColorAlloc{VK_NULL_HANDLE};

  // Define: Normal in View Space(sampler uv in clip space).
  VkImage gNormalImg{VK_NULL_HANDLE};
  VkImageView gNormalView{VK_NULL_HANDLE};
  VmaAllocation gNormalAlloc{VK_NULL_HANDLE};

  VkImage gDepth{VK_NULL_HANDLE};
  VkImageView gDepthView{VK_NULL_HANDLE};
  VmaAllocation gDepthAlloc{VK_NULL_HANDLE};

  // Define: pbr data (sampler uv in clip space).
  VkImage gPBRMaterial{VK_NULL_HANDLE};
  VkImageView gPBRMaterialView{VK_NULL_HANDLE};
  VmaAllocation gPBRMaterialAlloc{VK_NULL_HANDLE};
};

struct GBufferResources {
  std::array<GBufferObject, MAX_FRAMES_IN_FLIGHT> gBufferObjects;
  VkFormat gDepthFmt{VK_FORMAT_UNDEFINED};
  u_int32_t gDepthWidth{0};
  u_int32_t gDepthHeight{0};
  VkSampler gBufferSampler{VK_NULL_HANDLE};
};

class DeferedResourceManager : public RenderResourceBase {
 public:
  DeferedResourceManager() = default;
  ~DeferedResourceManager() = default;

  void Initialize(std::shared_ptr<RHI::VKRHI> rhi) override;
  void Destory(std::shared_ptr<RHI::VKRHI> rhi) override;

  VkImageView GetDepthImageView(int idx) const {
    return m_gb_obj[idx].gDepthView;
  }
  VkImageView GetPositionImageView(int idx) const {
    return m_gb_obj[idx].gPositionView;
  }
  VkImageView GetNormalImageView(int idx) const {
    return m_gb_obj[idx].gNormalView;
  }
  VkImageView GetAlbedoImageView(int idx) const {
    return m_gb_obj[idx].gColorView;
  }
  VkImageView GetPBRMaterialView(int idx) const {
    return m_gb_obj[idx].gPBRMaterialView;
  }

  VkFormat GetDepthFormat() const { return m_g_depth_fmt; }

  VkDescriptorImageInfo GetDepthDespImageInfo(int idx) const;
  VkDescriptorImageInfo GetPositionDespImageInfo(int idx) const;
  VkDescriptorImageInfo GetNormalDespImageInfo(int idx) const;
  VkDescriptorImageInfo GetAlbedoDespImageInfo(int idx) const;
  VkDescriptorImageInfo GetPBRMaterialImageInfo(int idx) const;

  VkWriteDescriptorSet GetDepthDespWrite() const;
  VkWriteDescriptorSet GetPositionDespWrite() const;
  VkWriteDescriptorSet GetNormalDespWrite() const;
  VkWriteDescriptorSet GetAlbedoDespWrite() const;
  VkWriteDescriptorSet GetPBRMaterialDespWrite() const;

 private:
  VkWriteDescriptorSet GetWrite() const;

  std::array<GBufferObject, MAX_FRAMES_IN_FLIGHT> m_gb_obj;
  VkFormat m_g_depth_fmt{VK_FORMAT_UNDEFINED};
  u_int32_t m_g_depth_width{0};
  u_int32_t m_g_depth_height{0};
  VkSampler m_g_sampler{VK_NULL_HANDLE};
};

}  // namespace ShaderStory

#endif