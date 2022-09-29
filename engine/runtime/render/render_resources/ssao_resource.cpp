#include "engine/runtime/render/render_resources/ssao_resource.hpp"

#include <random>

#include "engine/core/math.hpp"
#include "engine/runtime/render/rhi/vulkan/vk_utils.hpp"
#include "shaders/include/setting.h"

namespace ShaderStory {

SSAOResourceManager::SSAOResourceManager() {}

SSAOResourceManager::~SSAOResourceManager() {}

void SSAOResourceManager::Initialize(std::shared_ptr<RHI::VKRHI> rhi) {
  // image & view create.
  {
    VmaAllocationCreateInfo alloc_info{};
    alloc_info.usage = VMA_MEMORY_USAGE_AUTO;
    alloc_info.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
    alloc_info.priority = 1.0f;

    VkImageCreateInfo image_info{VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
    image_info.imageType = VK_IMAGE_TYPE_2D;
    image_info.format = VK_FORMAT_R8_UNORM;
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
                     &m_ssao_res[i].ssao_image, &m_ssao_res[i].ssao_alloc,
                     nullptr);

      view_info.image = m_ssao_res[i].ssao_image;

      vkCreateImageView(rhi->m_device, &view_info, nullptr,
                        &m_ssao_res[i].ssao_image_view);
    }
  }

  std::default_random_engine rnd_engine;
  std::uniform_real_distribution<float> rnd_dist(0.f, 1.f);
  // noise texture
  {
    std::vector<vec4> ssao_noise(SSAO_NOISE_DIM * SSAO_NOISE_DIM);

    for (size_t i = 0; i < ssao_noise.size(); ++i) {
      ssao_noise[i] = vec4(rnd_dist(rnd_engine) * 2.f - 1.f,
                           rnd_dist(rnd_engine) * 2.f - 1.f, 0.0, 0.0);
    }

    VmaAllocationCreateInfo alloc_info{};
    alloc_info.usage = VMA_MEMORY_USAGE_AUTO;
    alloc_info.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
    alloc_info.priority = 1.0f;

    VkImageCreateInfo info{VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
    info.imageType = VK_IMAGE_TYPE_2D;
    info.format = VK_FORMAT_R32G32B32A32_SFLOAT;
    info.extent.depth = 1;
    info.extent.width = SSAO_NOISE_DIM;
    info.extent.height = SSAO_NOISE_DIM;
    info.mipLevels = 1;
    info.arrayLayers = 1;
    info.samples = VK_SAMPLE_COUNT_1_BIT;
    info.tiling = VK_IMAGE_TILING_OPTIMAL;
    info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    info.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;

    vmaCreateImage(rhi->m_vma_allocator, &info, &alloc_info, &m_ssao_noise,
                   &m_ssao_noise_alloc, nullptr);

    VkImageViewCreateInfo view_info{VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
    view_info.image = m_ssao_noise;
    view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    view_info.format = VK_FORMAT_R32G32B32A32_SFLOAT;
    view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    view_info.subresourceRange.baseMipLevel = 0;
    view_info.subresourceRange.levelCount = 1;
    view_info.subresourceRange.baseArrayLayer = 0;
    view_info.subresourceRange.layerCount = 1;

    vkCreateImageView(rhi->m_device, &view_info, nullptr, &m_ssao_noise_view);

    // stage upload
    VkUtil::StageUploadImage(rhi, m_ssao_noise, SSAO_NOISE_DIM, SSAO_NOISE_DIM,
                             sizeof(vec4) * ssao_noise.size(), 1, 0,
                             VK_IMAGE_ASPECT_COLOR_BIT, ssao_noise.data());
  }

  // ssao kernal
  {
    std::vector<vec4> ssao_kernal(SSAO_KERNAL_SIZE);

    for (size_t i = 0; i < SSAO_KERNAL_SIZE; ++i) {
      vec3 sample =
          vec3(rnd_dist(rnd_engine) * 2.f - 1.f,
               rnd_dist(rnd_engine) * 2.f - 1.f, rnd_dist(rnd_engine));

      sample = normalize(sample);

      ssao_kernal[i] = vec4(sample, 0.0);
    }

    const size_t buf_sz = SSAO_KERNAL_SIZE * sizeof(vec4);

    VkBufferCreateInfo vert_buf_create_info{
        VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
    vert_buf_create_info.size = buf_sz;
    vert_buf_create_info.usage =
        VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    VmaAllocationCreateInfo vert_alloc_create_info{};
    vert_alloc_create_info.priority = 1.f;
    vert_alloc_create_info.usage = VMA_MEMORY_USAGE_AUTO;
    vert_alloc_create_info.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;

    vmaCreateBuffer(rhi->m_vma_allocator, &vert_buf_create_info,
                    &vert_alloc_create_info, &m_ssao_kernal_buf,
                    &m_ssao_kernal_alloc, nullptr);

    VkUtil::StageUploadBuffer(rhi, m_ssao_kernal_buf, buf_sz,
                              ssao_kernal.data());
  }

  // noise sampler
  {
    VkSamplerCreateInfo sampler_create_info{
        VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO};
    sampler_create_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    sampler_create_info.magFilter = VK_FILTER_NEAREST;  // minecraft is pixel!
    sampler_create_info.minFilter = VK_FILTER_NEAREST;
    sampler_create_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_create_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_create_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_create_info.anisotropyEnable = VK_TRUE;
    sampler_create_info.maxAnisotropy =
        rhi->m_pd_property.limits.maxSamplerAnisotropy;
    sampler_create_info.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    sampler_create_info.unnormalizedCoordinates = VK_FALSE;
    sampler_create_info.compareEnable = VK_FALSE;
    sampler_create_info.compareOp = VK_COMPARE_OP_ALWAYS;
    sampler_create_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

    VK_CHECK(vkCreateSampler(rhi->m_device, &sampler_create_info, nullptr,
                             &m_ssao_noise_sampler),
             "Create sampler failed.");
  }
}

void SSAOResourceManager::Destory(std::shared_ptr<RHI::VKRHI> rhi) {
  VkDevice device = rhi->m_device;
  VmaAllocator alloctor = rhi->m_vma_allocator;

  for (size_t i = 0; i < m_ssao_res.size(); ++i) {
    m_ssao_res[i].Destory(device, alloctor);
  }

  vmaDestroyBuffer(alloctor, m_ssao_kernal_buf, m_ssao_kernal_alloc);
}

VkDescriptorBufferInfo SSAOResourceManager::GetSSAOKernalBufInfo() const {
  VkDescriptorBufferInfo info{};
  info.buffer = m_ssao_kernal_buf;
  info.offset = 0;
  info.range = SSAO_KERNAL_SIZE * sizeof(vec4);
  return info;
}

VkWriteDescriptorSet SSAOResourceManager::GetSSAOKernalWriteInfo() const {
  VkWriteDescriptorSet write{};
  write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  write.descriptorCount = 1;
  write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
  write.dstArrayElement = 0;
  write.pImageInfo = nullptr;
  write.pTexelBufferView = nullptr;
  write.pNext = nullptr;
  return write;
}

std::array<VkWriteDescriptorSet, MAX_FRAMES_IN_FLIGHT>
SSAOResourceManager::GetDespWrtiers() const {
  assert(false);
  std::array<VkWriteDescriptorSet, MAX_FRAMES_IN_FLIGHT> writers;

  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
    VkDescriptorImageInfo info{};
    // TODO: BUG
    info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    info.sampler = nullptr;
    info.imageView = m_ssao_res[i].ssao_image_view;

    writers[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writers[i].pNext = nullptr;
    writers[i].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    writers[i].descriptorCount = 1;
    // writers[i].pImageInfo = &info;
  }

  return writers;
}

VkDescriptorImageInfo SSAOResourceManager::GetSSAONoiseImageInfo() const {
  VkDescriptorImageInfo info{};
  info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
  info.imageView = m_ssao_noise_view;
  info.sampler = m_ssao_noise_sampler;
  return info;
}

VkWriteDescriptorSet SSAOResourceManager::GetSSAONoiseWriteInfo() const {
  VkWriteDescriptorSet write{};
  write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  write.descriptorCount = 1;
  write.dstArrayElement = 0;
  write.pBufferInfo = nullptr;
  write.pTexelBufferView = nullptr;
  write.pNext = nullptr;
  return write;
}

VkDescriptorImageInfo SSAOResourceManager::GetSSAOTextureImageInfo(
    int idx) const {
  VkDescriptorImageInfo info{};
  info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
  info.imageView = m_ssao_res[idx].ssao_image_view;
  info.sampler = m_ssao_noise_sampler;  // TODO: rename sampler.
  return info;
}

VkWriteDescriptorSet SSAOResourceManager::GetSSAOWTextureWrite() const {
  VkWriteDescriptorSet write{};
  write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  write.descriptorCount = 1;
  write.dstArrayElement = 0;
  write.pBufferInfo = nullptr;
  write.pTexelBufferView = nullptr;
  write.pNext = nullptr;
  return write;
}

}  // namespace ShaderStory