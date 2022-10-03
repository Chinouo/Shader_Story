#ifndef MATERIALS_HPP
#define MATERIALS_HPP

#include "engine/runtime/render/render_base.hpp"

namespace ShaderStory {

struct TerrainTexture {
  VkImage image{VK_NULL_HANDLE};
  VkImageView view{VK_NULL_HANDLE};
  VmaAllocation alloc{VK_NULL_HANDLE};

  static VkImageCreateInfo GetDefaultImageCreateInfo() {
    VkImageCreateInfo info{VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
    info.imageType = VK_IMAGE_TYPE_2D;
    info.extent.depth = 1;
    info.mipLevels = 1;
    info.arrayLayers = 1;
    info.samples = VK_SAMPLE_COUNT_1_BIT;
    info.tiling = VK_IMAGE_TILING_OPTIMAL;
    info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    info.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    return info;
  }

  static VkImageViewCreateInfo GetDefaultImageViewInfo() {
    VkImageViewCreateInfo view_info{VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
    view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    view_info.subresourceRange.baseMipLevel = 0;
    view_info.subresourceRange.levelCount = 1;
    view_info.subresourceRange.baseArrayLayer = 0;
    view_info.subresourceRange.layerCount = 1;
    return view_info;
  }
};

class MaterialManager : public RenderResourceBase {
 public:
  MaterialManager() = default;
  ~MaterialManager() = default;

  void Initialize(std::shared_ptr<RHI::VKRHI> rhi) override;
  void Destory(std::shared_ptr<RHI::VKRHI> rhi) override;

  VkWriteDescriptorSet GetAlbedoImageWrite() const;
  VkDescriptorImageInfo GetAlbedoDespImageInfo() const;
  VkWriteDescriptorSet GetNormalMapImageWrite() const;
  VkDescriptorImageInfo GetNormalMapDespImageInfo() const;

  VkWriteDescriptorSet GetMaterialDespWrite() const;
  VkDescriptorImageInfo GetMaterialDespImageInfo() const;

  // VkWriteDescriptorSet GetMetallicImageWrite() const;
  // VkDescriptorImageInfo GetMetallicDespImageInfo() const;
  // VkWriteDescriptorSet GetRoughnessImageWrite() const;
  // VkDescriptorImageInfo GetRoughnesspImageInfo() const;
  // VkWriteDescriptorSet GetEmissiveImageWrite() const;
  // VkDescriptorImageInfo GetEmissivepImageInfo() const;

 private:
  VkWriteDescriptorSet GetWrite() const;

  void LoadAlbedoTexture(const std::shared_ptr<RHI::VKRHI>& rhi,
                         const std::string& file);

  void LoadNormalMapTexture(const std::shared_ptr<RHI::VKRHI>& rhi,
                            const std::string& file);

  void LoadMaterialTexture(const std::shared_ptr<RHI::VKRHI>& rhi,
                           const std::string& r, const std::string& m,
                           const std::string& e);

  void LoadRoughnessTexture(const std::shared_ptr<RHI::VKRHI>& rhi,
                            const std::string& file);

  void LoadMetallicTexture(const std::shared_ptr<RHI::VKRHI>& rhi,
                           const std::string& file);

  void LoadEmissiveTexture(const std::shared_ptr<RHI::VKRHI>& rhi,
                           const std::string& file);

  TerrainTexture m_terrain_albedo;
  TerrainTexture m_terrain_normalmap;
  // TerrainTexture m_terrain_metallic;
  // TerrainTexture m_terrain_roughness;
  // TerrainTexture m_terrain_emissive;

  // packed roughness/metallic/emissive into single texture.
  TerrainTexture m_terrain_material;
};

}  // namespace ShaderStory

#endif