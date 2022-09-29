#include "engine/runtime/render/render_resources/defered_resource.hpp"

namespace ShaderStory {

void DeferedResourceManager::Initialize(std::shared_ptr<RHI::VKRHI> rhi) {
  auto& g_buffer_objects = m_gb_obj;
  // G-Position
  {
    VmaAllocationCreateInfo alloc_info{};
    alloc_info.usage = VMA_MEMORY_USAGE_AUTO;
    alloc_info.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
    alloc_info.priority = 1.0f;

    VkImageCreateInfo image_info{VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
    image_info.imageType = VK_IMAGE_TYPE_2D;
    image_info.format = VK_FORMAT_R16G16B16A16_SFLOAT;
    image_info.arrayLayers = 1;
    image_info.extent.width = rhi->m_swapchain_extent.width;
    image_info.extent.height = rhi->m_swapchain_extent.height;
    image_info.extent.depth = 1;
    image_info.mipLevels = 1;
    image_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_info.usage =
        VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    VkImageViewCreateInfo view_info{VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
    view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    view_info.format = image_info.format;
    view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    view_info.subresourceRange.baseMipLevel = 0;
    view_info.subresourceRange.levelCount = 1;
    view_info.subresourceRange.baseArrayLayer = 0;
    view_info.subresourceRange.layerCount = 1;

    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
      vmaCreateImage(rhi->m_vma_allocator, &image_info, &alloc_info,
                     &g_buffer_objects[i].gPositionImg,
                     &g_buffer_objects[i].gPositionAlloc, nullptr);

      view_info.image = g_buffer_objects[i].gPositionImg;

      vkCreateImageView(rhi->m_device, &view_info, nullptr,
                        &g_buffer_objects[i].gPositionView);
    }
  }

  // G-Normal
  {
    VmaAllocationCreateInfo alloc_info{};
    alloc_info.usage = VMA_MEMORY_USAGE_AUTO;
    alloc_info.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
    alloc_info.priority = 1.0f;

    VkImageCreateInfo image_info{VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
    image_info.imageType = VK_IMAGE_TYPE_2D;
    image_info.format = VK_FORMAT_R16G16B16A16_SFLOAT;
    image_info.arrayLayers = 1;
    image_info.extent.width = rhi->m_swapchain_extent.width;
    image_info.extent.height = rhi->m_swapchain_extent.height;
    image_info.extent.depth = 1;
    image_info.mipLevels = 1;
    image_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_info.usage =
        VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    VkImageViewCreateInfo view_info{VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
    view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    view_info.format = image_info.format;
    view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    view_info.subresourceRange.baseMipLevel = 0;
    view_info.subresourceRange.levelCount = 1;
    view_info.subresourceRange.baseArrayLayer = 0;
    view_info.subresourceRange.layerCount = 1;

    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
      vmaCreateImage(rhi->m_vma_allocator, &image_info, &alloc_info,
                     &g_buffer_objects[i].gNormalImg,
                     &g_buffer_objects[i].gNormalAlloc, nullptr);

      view_info.image = g_buffer_objects[i].gNormalImg;

      vkCreateImageView(rhi->m_device, &view_info, nullptr,
                        &g_buffer_objects[i].gNormalView);
    }
  }

  // G-Albedo
  {
    VmaAllocationCreateInfo alloc_info{};
    alloc_info.usage = VMA_MEMORY_USAGE_AUTO;
    alloc_info.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
    alloc_info.priority = 1.0f;

    VkImageCreateInfo image_info{VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
    image_info.imageType = VK_IMAGE_TYPE_2D;
    image_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_info.arrayLayers = 1;
    image_info.extent.width = rhi->m_swapchain_extent.width;
    image_info.extent.height = rhi->m_swapchain_extent.height;
    image_info.extent.depth = 1;
    image_info.mipLevels = 1;
    image_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_info.usage =
        VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    VkImageViewCreateInfo view_info{VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
    view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    view_info.format = image_info.format;
    view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    view_info.subresourceRange.baseMipLevel = 0;
    view_info.subresourceRange.levelCount = 1;
    view_info.subresourceRange.baseArrayLayer = 0;
    view_info.subresourceRange.layerCount = 1;

    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
      vmaCreateImage(rhi->m_vma_allocator, &image_info, &alloc_info,
                     &g_buffer_objects[i].gColorImg,
                     &g_buffer_objects[i].gColorAlloc, nullptr);

      view_info.image = g_buffer_objects[i].gColorImg;

      vkCreateImageView(rhi->m_device, &view_info, nullptr,
                        &g_buffer_objects[i].gColorView);
    }
  }

  // G-Depth
  {
    VkFormat required_format = rhi->FindSupportFormat(
        {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT,
         VK_FORMAT_D24_UNORM_S8_UINT},
        VK_IMAGE_TILING_OPTIMAL,
        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);

    m_g_depth_fmt = required_format;
    m_g_depth_width = rhi->m_swapchain_extent.width;
    m_g_depth_height = rhi->m_swapchain_extent.height;

    VkImageCreateInfo image_info{VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
    image_info.format = required_format;
    image_info.imageType = VK_IMAGE_TYPE_2D;
    image_info.extent.width = m_g_depth_width;
    image_info.extent.height = m_g_depth_height;
    image_info.extent.depth = 1;
    image_info.mipLevels = 1;
    image_info.arrayLayers = 1;
    image_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_info.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT |
                       VK_IMAGE_USAGE_SAMPLED_BIT;
    image_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VkImageViewCreateInfo view_info{VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
    view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    view_info.format = image_info.format;
    view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    view_info.subresourceRange.baseMipLevel = 0;
    view_info.subresourceRange.levelCount = 1;
    view_info.subresourceRange.baseArrayLayer = 0;
    view_info.subresourceRange.layerCount = 1;

    VmaAllocationCreateInfo alloc_info{};
    alloc_info.usage = VMA_MEMORY_USAGE_AUTO;
    alloc_info.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
    alloc_info.priority = 1.0f;

    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
      vmaCreateImage(rhi->m_vma_allocator, &image_info, &alloc_info,
                     &g_buffer_objects[i].gDepth,
                     &g_buffer_objects[i].gDepthAlloc, nullptr);

      view_info.image = g_buffer_objects[i].gDepth;

      vkCreateImageView(rhi->m_device, &view_info, nullptr,
                        &g_buffer_objects[i].gDepthView);
    }
  }

  // g-buffer sampler
  VkSamplerCreateInfo sampler_info{VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO};
  sampler_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
  sampler_info.magFilter = VK_FILTER_NEAREST;
  sampler_info.minFilter = VK_FILTER_NEAREST;
  sampler_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  sampler_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  sampler_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  sampler_info.anisotropyEnable = VK_TRUE;
  sampler_info.maxAnisotropy = rhi->m_pd_property.limits.maxSamplerAnisotropy;
  sampler_info.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
  sampler_info.unnormalizedCoordinates = VK_FALSE;
  sampler_info.compareEnable = VK_FALSE;
  sampler_info.compareOp = VK_COMPARE_OP_ALWAYS;
  sampler_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

  VK_CHECK(vkCreateSampler(rhi->m_device, &sampler_info, nullptr, &m_g_sampler),
           "Create sampler failed.");
}

void DeferedResourceManager::Destory(std::shared_ptr<RHI::VKRHI> rhi) {
  auto allocator = rhi->m_vma_allocator;
  auto device = rhi->m_device;
  for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
    vmaDestroyImage(allocator, m_gb_obj[i].gColorImg, m_gb_obj[i].gColorAlloc);
    vkDestroyImageView(device, m_gb_obj[i].gColorView, nullptr);

    vmaDestroyImage(allocator, m_gb_obj[i].gNormalImg,
                    m_gb_obj[i].gNormalAlloc);
    vkDestroyImageView(device, m_gb_obj[i].gNormalView, nullptr);

    vmaDestroyImage(allocator, m_gb_obj[i].gPositionImg,
                    m_gb_obj[i].gPositionAlloc);
    vkDestroyImageView(device, m_gb_obj[i].gPositionView, nullptr);

    vmaDestroyImage(allocator, m_gb_obj[i].gDepth, m_gb_obj[i].gDepthAlloc);
    vkDestroyImageView(device, m_gb_obj[i].gDepthView, nullptr);
  }

  vkDestroySampler(device, m_g_sampler, nullptr);
}

VkDescriptorImageInfo DeferedResourceManager::GetDepthDespImageInfo(
    int idx) const {
  VkDescriptorImageInfo info{};
  info.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
  info.sampler = m_g_sampler;
  info.imageView = m_gb_obj[idx].gDepthView;
  return info;
}

VkDescriptorImageInfo DeferedResourceManager::GetPositionDespImageInfo(
    int idx) const {
  VkDescriptorImageInfo info{};
  info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
  info.sampler = m_g_sampler;
  info.imageView = m_gb_obj[idx].gPositionView;
  return info;
}

VkDescriptorImageInfo DeferedResourceManager::GetNormalDespImageInfo(
    int idx) const {
  VkDescriptorImageInfo info{};
  info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
  info.sampler = m_g_sampler;
  info.imageView = m_gb_obj[idx].gNormalView;
  return info;
}

VkDescriptorImageInfo DeferedResourceManager::GetAlbedoDespImageInfo(
    int idx) const {
  VkDescriptorImageInfo info{};
  info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
  info.sampler = m_g_sampler;
  info.imageView = m_gb_obj[idx].gColorView;
  return info;
}

VkWriteDescriptorSet DeferedResourceManager::GetWrite() const {
  VkWriteDescriptorSet write{VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
  write.pNext = nullptr;
  write.dstArrayElement = 0;
  write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  write.descriptorCount = 1;
  return write;
}

VkWriteDescriptorSet DeferedResourceManager::GetDepthDespWrite() const {
  return GetWrite();
}

VkWriteDescriptorSet DeferedResourceManager::GetPositionDespWrite() const {
  return GetWrite();
}

VkWriteDescriptorSet DeferedResourceManager::GetNormalDespWrite() const {
  return GetWrite();
}

VkWriteDescriptorSet DeferedResourceManager::GetAlbedoDespWrite() const {
  return GetWrite();
}

};  // namespace ShaderStory