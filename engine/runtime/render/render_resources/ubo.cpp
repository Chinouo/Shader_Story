#include "engine/runtime/render/render_resources/ubo.hpp"

namespace ShaderStory {

UniformObjectManager::UniformObjectManager() {}

UniformObjectManager::~UniformObjectManager() {}

void UniformObjectManager::Initialize(std::shared_ptr<RHI::VKRHI> rhi) {
  // ubo create and alloc
  {
    VkBufferCreateInfo buf_create_info{VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
    buf_create_info.size = GetPerframeUBODynamicOffset() * MAX_FRAMES_IN_FLIGHT;
    buf_create_info.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;

    VmaAllocationCreateInfo alloc_create_info{};
    alloc_create_info.usage = VMA_MEMORY_USAGE_AUTO;
    alloc_create_info.priority = 1.0f;
    alloc_create_info.flags =
        VMA_ALLOCATION_CREATE_MAPPED_BIT |
        VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
        VMA_ALLOCATION_CREATE_HOST_ACCESS_ALLOW_TRANSFER_INSTEAD_BIT;

    vmaCreateBufferWithAlignment(
        rhi->m_vma_allocator, &buf_create_info, &alloc_create_info,
        m_perframe_ubo.min_algiment, &m_perframe_ubo.perframe_data_buffer,
        &m_perframe_ubo.perframe_data_alloc,
        &m_perframe_ubo.perframe_data_alloc_info);

    VkMemoryPropertyFlags memPropFlags;
    vmaGetAllocationMemoryProperties(rhi->m_vma_allocator,
                                     m_perframe_ubo.perframe_data_alloc,
                                     &memPropFlags);

    ASSERT(memPropFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT &&
           "Allocate Perframe Uniform Buffer Failed.");

    vmaMapMemory(rhi->m_vma_allocator, m_perframe_ubo.perframe_data_alloc,
                 &m_perframe_ubo.mapped_memory);

    ASSERT(m_perframe_ubo.perframe_data_alloc_info.pMappedData ==
           m_perframe_ubo.mapped_memory);
  }
}

void UniformObjectManager::UpdateCurrentFrameData(const PerframeDataUBO& data,
                                                  int current_frame_idx) {
  void* dst_with_offset = (char*)m_perframe_ubo.mapped_memory +
                          current_frame_idx * GetPerframeUBODynamicOffset();

  memcpy(dst_with_offset, &data, sizeof(PerframeDataUBO));
}

void UniformObjectManager::Destory(std::shared_ptr<RHI::VKRHI> rhi) {
  VkDevice device = rhi->m_device;
  VmaAllocator allocator = rhi->m_vma_allocator;
  m_perframe_ubo.mapped_memory = nullptr;
  vmaUnmapMemory(allocator, m_perframe_ubo.perframe_data_alloc);
  vmaDestroyBuffer(allocator, m_perframe_ubo.perframe_data_buffer,
                   m_perframe_ubo.perframe_data_alloc);
}

VkDescriptorBufferInfo UniformObjectManager::GetDespBufInfo() const {
  VkDescriptorBufferInfo info{};
  info.offset = 0;
  info.range = sizeof(PerframeDataUBO);
  info.buffer = m_perframe_ubo.perframe_data_buffer;
  return info;
}

VkWriteDescriptorSet UniformObjectManager::GetDespWrite() const {
  VkWriteDescriptorSet write{};
  write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  write.pNext = nullptr;
  write.dstArrayElement = 0;
  write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
  write.descriptorCount = 1;

  return write;
}

}  // namespace ShaderStory