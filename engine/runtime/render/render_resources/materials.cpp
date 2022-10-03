#include "engine/runtime/render/render_resources/materials.hpp"

#include <iostream>
#include <string>
#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include "third_party/stb/stb_image.h"
#endif
#include "engine/core/math.hpp"
#include "engine/runtime/render/rhi/vulkan/vk_utils.hpp"
namespace ShaderStory {

void MaterialManager::Initialize(std::shared_ptr<RHI::VKRHI> rhi) {
  LoadAlbedoTexture(rhi, "assets/o-RGBA.png");
  LoadNormalMapTexture(rhi, "assets/oN-RGBA.png");
  LoadMaterialTexture(rhi, "assets/oR-RGBA.png", "assets/oM-RGBA.png",
                      "assets/oE-RGBA.png");
}

void MaterialManager::Destory(std::shared_ptr<RHI::VKRHI> rhi) {
  VkDevice device = rhi->m_device;
  VmaAllocator allocator = rhi->m_vma_allocator;
  vmaDestroyImage(allocator, m_terrain_albedo.image, m_terrain_albedo.alloc);
  vkDestroyImageView(device, m_terrain_albedo.view, nullptr);
}

void MaterialManager::LoadAlbedoTexture(const std::shared_ptr<RHI::VKRHI>& rhi,
                                        const std::string& file) {
  int w = 0, h = 0;
  int channels;
  stbi_uc* img = stbi_load(file.c_str(), &w, &h, &channels, STBI_rgb_alpha);

  VmaAllocationCreateInfo alloc_info{};
  alloc_info.usage = VMA_MEMORY_USAGE_AUTO;
  alloc_info.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
  alloc_info.priority = 1.0f;

  VkImageCreateInfo image_info = TerrainTexture::GetDefaultImageCreateInfo();
  image_info.extent.width = w;
  image_info.extent.height = h;
  image_info.format = VK_FORMAT_R8G8B8A8_SRGB;

  vmaCreateImage(rhi->m_vma_allocator, &image_info, &alloc_info,
                 &m_terrain_albedo.image, &m_terrain_albedo.alloc, nullptr);

  VkImageViewCreateInfo view_info = TerrainTexture::GetDefaultImageViewInfo();
  view_info.image = m_terrain_albedo.image;
  view_info.format = image_info.format;
  view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;

  vkCreateImageView(rhi->m_device, &view_info, nullptr, &m_terrain_albedo.view);

  VkUtil::StageUploadImage(rhi, m_terrain_albedo.image, w, h, w * h * 4, 1, 0,
                           view_info.subresourceRange.aspectMask, img);

  stbi_image_free(img);
}

void MaterialManager::LoadNormalMapTexture(
    const std::shared_ptr<RHI::VKRHI>& rhi, const std::string& file) {
  int w = 0, h = 0;
  int channels;
  stbi_uc* img = stbi_load(file.c_str(), &w, &h, &channels, STBI_rgb_alpha);

  VmaAllocationCreateInfo alloc_info{};
  alloc_info.usage = VMA_MEMORY_USAGE_AUTO;
  alloc_info.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
  alloc_info.priority = 1.0f;

  VkImageCreateInfo image_info = TerrainTexture::GetDefaultImageCreateInfo();
  image_info.extent.width = w;
  image_info.extent.height = h;
  image_info.format = VK_FORMAT_R8G8B8A8_SRGB;

  vmaCreateImage(rhi->m_vma_allocator, &image_info, &alloc_info,
                 &m_terrain_normalmap.image, &m_terrain_normalmap.alloc,
                 nullptr);

  VkImageViewCreateInfo view_info = TerrainTexture::GetDefaultImageViewInfo();
  view_info.image = m_terrain_normalmap.image;
  view_info.format = image_info.format;
  view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;

  vkCreateImageView(rhi->m_device, &view_info, nullptr,
                    &m_terrain_normalmap.view);

  VkUtil::StageUploadImage(rhi, m_terrain_normalmap.image, w, h,
                           w * h * sizeof(u_int32_t), 1, 0,
                           view_info.subresourceRange.aspectMask, img);

  stbi_image_free(img);
}

void MaterialManager::LoadMaterialTexture(
    const std::shared_ptr<RHI::VKRHI>& rhi, const std::string& r,
    const std::string& m, const std::string& e) {
  // RGBA:R: roughness G: metallic B: emissive
  int w = 0, h = 0;
  int channels;
  // all have the same size.
  stbi_uc* roughness = stbi_load(r.c_str(), &w, &h, &channels, STBI_grey);
  stbi_uc* metallic = stbi_load(m.c_str(), &w, &h, &channels, STBI_grey);
  stbi_uc* emissive = stbi_load(e.c_str(), &w, &h, &channels, STBI_grey);

  std::vector<vec4> material;
  material.resize(w * h);
  // 0 - 255
  const float div = 1.0 / 255.0;
  for (int i = 0; i < material.size(); ++i) {
    material[i] = div * vec4((float)roughness[i], (float)metallic[i],
                             (float)emissive[i], 0.f);
  }
  stbi_image_free(roughness);
  stbi_image_free(metallic);
  stbi_image_free(emissive);

  VmaAllocationCreateInfo alloc_info{};
  alloc_info.usage = VMA_MEMORY_USAGE_AUTO;
  alloc_info.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
  alloc_info.priority = 1.0f;

  VkImageCreateInfo image_info = TerrainTexture::GetDefaultImageCreateInfo();
  image_info.extent.width = w;
  image_info.extent.height = h;
  image_info.format = VK_FORMAT_R32G32B32A32_SFLOAT;

  VK_CHECK(vmaCreateImage(rhi->m_vma_allocator, &image_info, &alloc_info,
                          &m_terrain_material.image, &m_terrain_material.alloc,
                          nullptr),
           "Failed to create material texture.");

  VkImageViewCreateInfo view_info = TerrainTexture::GetDefaultImageViewInfo();
  view_info.image = m_terrain_material.image;
  view_info.format = image_info.format;
  view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;

  vkCreateImageView(rhi->m_device, &view_info, nullptr,
                    &m_terrain_material.view);

  VkUtil::StageUploadImage(
      rhi, m_terrain_material.image, w, h, w * h * sizeof(vec4), 1, 0,
      view_info.subresourceRange.aspectMask, material.data());
}

// void MaterialManager::LoadMetallicTexture(
//     const std::shared_ptr<RHI::VKRHI>& rhi, const std::string& file) {
//   int w = 0, h = 0;
//   int channels;
//   stbi_uc* img = stbi_load(file.c_str(), &w, &h, &channels, STBI_grey);

//   VmaAllocationCreateInfo alloc_info{};
//   alloc_info.usage = VMA_MEMORY_USAGE_AUTO;
//   alloc_info.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
//   alloc_info.priority = 1.0f;

//   VkImageCreateInfo image_info = TerrainTexture::GetDefaultImageCreateInfo();
//   image_info.extent.width = w;
//   image_info.extent.height = h;
//   image_info.format = VK_FORMAT_R8_UNORM;

//   vmaCreateImage(rhi->m_vma_allocator, &image_info, &alloc_info,
//                  &m_terrain_albedo.image, &m_terrain_albedo.alloc, nullptr);

//   VkImageViewCreateInfo view_info =
//   TerrainTexture::GetDefaultImageViewInfo(); view_info.image =
//   m_terrain_albedo.image; view_info.format = image_info.format;
//   view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;

//   vkCreateImageView(rhi->m_device, &view_info, nullptr,
//   &m_terrain_albedo.view);

//   VkUtil::StageUploadImage(rhi, m_terrain_albedo.image, w, h,
//   sizeof(u_int32_t),
//                            image_info.arrayLayers, image_info.mipLevels,
//                            view_info.subresourceRange.aspectMask, img);

//   stbi_image_free(img);
// }

// void MaterialManager::LoadRoughnessTexture(
//     const std::shared_ptr<RHI::VKRHI>& rhi, const std::string& file) {
//   int w = 0, h = 0;
//   int channels;
//   stbi_uc* img = stbi_load(file.c_str(), &w, &h, &channels, STBI_grey);

//   VmaAllocationCreateInfo alloc_info{};
//   alloc_info.usage = VMA_MEMORY_USAGE_AUTO;
//   alloc_info.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
//   alloc_info.priority = 1.0f;

//   VkImageCreateInfo image_info = TerrainTexture::GetDefaultImageCreateInfo();
//   image_info.extent.width = w;
//   image_info.extent.height = h;
//   image_info.format = VK_FORMAT_R8_UNORM;

//   vmaCreateImage(rhi->m_vma_allocator, &image_info, &alloc_info,
//                  &m_terrain_albedo.image, &m_terrain_albedo.alloc, nullptr);

//   VkImageViewCreateInfo view_info =
//   TerrainTexture::GetDefaultImageViewInfo(); view_info.image =
//   m_terrain_albedo.image; view_info.format = image_info.format;
//   view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;

//   vkCreateImageView(rhi->m_device, &view_info, nullptr,
//   &m_terrain_albedo.view);

//   VkUtil::StageUploadImage(rhi, m_terrain_albedo.image, w, h,
//   sizeof(u_int32_t),
//                            image_info.arrayLayers, image_info.mipLevels,
//                            view_info.subresourceRange.aspectMask, img);

//   stbi_image_free(img);
// }

// void MaterialManager::LoadEmissiveTexture(
//     const std::shared_ptr<RHI::VKRHI>& rhi, const std::string& file) {
//   int w = 0, h = 0;
//   int channels;
//   stbi_uc* img = stbi_load(file.c_str(), &w, &h, &channels, STBI_grey);

//   VmaAllocationCreateInfo alloc_info{};
//   alloc_info.usage = VMA_MEMORY_USAGE_AUTO;
//   alloc_info.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
//   alloc_info.priority = 1.0f;

//   VkImageCreateInfo image_info = TerrainTexture::GetDefaultImageCreateInfo();
//   image_info.extent.width = w;
//   image_info.extent.height = h;
//   image_info.format = VK_FORMAT_R8_UNORM;

//   vmaCreateImage(rhi->m_vma_allocator, &image_info, &alloc_info,
//                  &m_terrain_albedo.image, &m_terrain_albedo.alloc, nullptr);

//   VkImageViewCreateInfo view_info =
//   TerrainTexture::GetDefaultImageViewInfo(); view_info.image =
//   m_terrain_albedo.image; view_info.format = image_info.format;
//   view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;

//   vkCreateImageView(rhi->m_device, &view_info, nullptr,
//   &m_terrain_albedo.view);

//   VkUtil::StageUploadImage(rhi, m_terrain_albedo.image, w, h,
//   sizeof(u_int32_t),
//                            image_info.arrayLayers, image_info.mipLevels,
//                            view_info.subresourceRange.aspectMask, img);

//   stbi_image_free(img);
// }

VkWriteDescriptorSet MaterialManager::GetAlbedoImageWrite() const {
  return GetWrite();
}

VkWriteDescriptorSet MaterialManager::GetNormalMapImageWrite() const {
  return GetWrite();
}

VkWriteDescriptorSet MaterialManager::GetMaterialDespWrite() const {
  return GetWrite();
}

VkDescriptorImageInfo MaterialManager::GetMaterialDespImageInfo() const {
  VkDescriptorImageInfo info{};
  info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
  info.imageView = m_terrain_material.view;
  info.sampler = nullptr;  // TODO:
  return info;
}

// VkWriteDescriptorSet MaterialManager::GetMetallicImageWrite() const {
//   return GetWrite();
// }

// VkWriteDescriptorSet MaterialManager::GetRoughnessImageWrite() const {
//   return GetWrite();
// }

// VkWriteDescriptorSet MaterialManager::GetEmissiveImageWrite() const {
//   return GetWrite();
// }

VkDescriptorImageInfo MaterialManager::GetAlbedoDespImageInfo() const {
  VkDescriptorImageInfo info{};
  info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
  info.imageView = m_terrain_albedo.view;
  info.sampler = nullptr;  // TODO:
  return info;
}

VkDescriptorImageInfo MaterialManager::GetNormalMapDespImageInfo() const {
  VkDescriptorImageInfo info{};
  info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
  info.imageView = m_terrain_normalmap.view;
  info.sampler = nullptr;  // TODO:
  return info;
}

// VkDescriptorImageInfo MaterialManager::GetMetallicDespImageInfo() const {
//   VkDescriptorImageInfo info{};
//   info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
//   info.imageView = m_terrain_metallic.view;
//   info.sampler = nullptr;  // TODO:
//   return info;
// }

// VkDescriptorImageInfo MaterialManager::GetRoughnesspImageInfo() const {
//   VkDescriptorImageInfo info{};
//   info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
//   info.imageView = m_terrain_roughness.view;
//   info.sampler = nullptr;  // TODO:
//   return info;
// }

// VkDescriptorImageInfo MaterialManager::GetEmissivepImageInfo() const {
//   VkDescriptorImageInfo info{};
//   info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
//   info.imageView = m_terrain_emissive.view;
//   info.sampler = nullptr;  // TODO:
//   return info;
// }

VkWriteDescriptorSet MaterialManager::GetWrite() const {
  VkWriteDescriptorSet write{VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
  write.pNext = nullptr;
  write.dstArrayElement = 0;
  write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  write.descriptorCount = 1;
  return write;
}

}  // namespace ShaderStory
