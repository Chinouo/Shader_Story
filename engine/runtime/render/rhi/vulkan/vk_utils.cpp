#include "engine/runtime/render/rhi/vulkan/vk_utils.hpp"

#include <exception>
#include <fstream>

#include "engine/common/macros.h"
namespace ShaderStory {
shaderc::Compiler VkUtil::compiler;

shaderc::CompileOptions compile_op;

std::vector<u_int32_t> VkUtil::GetSpirvBinary(const std::string& file,
                                              shaderc_shader_kind shader_type) {
  std::fstream in(file, std::ios::in);
  ASSERT(in.is_open() && "Failed to open file");
  in.seekg(0, std::ios::end);  // 从末尾开始计算偏移量
  std::streampos size = in.tellg();
  in.seekg(0, std::ios::beg);  // 从起始位置开始计算偏移量
  std::vector<char> buf(size);
  in.read(buf.data(), size);
  in.close();
  ASSERT(!buf.empty());

  auto result = compiler.CompileGlslToSpv(buf.data(), buf.size(), shader_type,
                                          file.c_str());

  if (!result.GetErrorMessage().empty()) {
    throw std::runtime_error(result.GetErrorMessage());
  }

  return {result.cbegin(), result.cend()};
}

VkShaderModule VkUtil::RuntimeCreateShaderModule(
    VkDevice device, const std::string& file, shaderc_shader_kind shader_type) {
  std::vector<uint32_t> spirv(GetSpirvBinary(file, shader_type));

  VkShaderModuleCreateInfo info{};
  info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
  info.codeSize = spirv.size() * sizeof(uint32_t);
  info.pCode = spirv.data();

  VkShaderModule ret;

  VK_CHECK(vkCreateShaderModule(device, &info, nullptr, &ret),
           "Failed to create shadermodule.");

  return ret;
}

uint32_t VkUtil::FindMemoryType(
    const VkPhysicalDeviceMemoryProperties& properties, uint32_t type_filter,
    VkMemoryPropertyFlags flag) {
  for (uint32_t i = 0; i < properties.memoryTypeCount; i++) {
    if (type_filter & (1 << i) &&
        (properties.memoryTypes[i].propertyFlags & flag) == flag) {
      return i;
    }
  }
  throw std::runtime_error("Failed to find memory type.");
}

VkImageView VkUtil::CreateImageView(VkDevice device, VkImage& image,
                                    VkFormat format,
                                    VkImageAspectFlags image_aspect_flags,
                                    VkImageViewType view_type,
                                    uint32_t layout_count, uint32_t miplevels) {
  VkImageView image_view;

  VkImageViewCreateInfo info{VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
  info.image = image;
  info.viewType = view_type;
  info.format = format;
  info.subresourceRange.aspectMask = image_aspect_flags;
  info.subresourceRange.baseMipLevel = 0;
  info.subresourceRange.levelCount = miplevels;
  info.subresourceRange.baseArrayLayer = 0;
  info.subresourceRange.layerCount = layout_count;

  VK_CHECK(vkCreateImageView(device, &info, nullptr, &image_view),
           "Failed to create imageview.");
  return image_view;
}

void VkUtil::TransitionImageLayout(VkImage image, VkFormat format,
                                   VkImageLayout old_layout,
                                   VkImageLayout new_layout) {}

void VkUtil::StageUploadBuffer(std::shared_ptr<RHI::VKRHI> rhi, VkBuffer dst,
                               VkDeviceSize size, const void* data) {
  ASSERT(dst);
  // staging buffer create and upload.
  VkBuffer stage_buf;
  VkBufferCreateInfo stage_buffer_create_info{
      VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
  stage_buffer_create_info.size = size;
  stage_buffer_create_info.usage =
      VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
  VmaAllocationCreateInfo stage_create_info{};
  stage_create_info.usage = VMA_MEMORY_USAGE_AUTO;
  stage_create_info.flags =
      VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
      VMA_ALLOCATION_CREATE_MAPPED_BIT;
  VmaAllocation stage_alloc;
  VmaAllocationInfo stage_alloc_info;
  vmaCreateBuffer(rhi->m_vma_allocator, &stage_buffer_create_info,
                  &stage_create_info, &stage_buf, &stage_alloc,
                  &stage_alloc_info);

  memcpy(stage_alloc_info.pMappedData, data, stage_buffer_create_info.size);

  rhi->CopyBuffer(stage_buf, 0, dst, 0, size);

  // free staging buffer.
  vmaDestroyBuffer(rhi->m_vma_allocator, stage_buf, stage_alloc);
}

void VkUtil::StageUploadImage(std::shared_ptr<RHI::VKRHI> rhi, VkImage dst,
                              u_int32_t width, u_int32_t height,
                              VkDeviceSize size, uint32_t layer_count,
                              uint32_t miplevels,
                              VkImageAspectFlags aspect_mask_bits,
                              const void* data) {
  // copy image data to buffer.
  VkBuffer stage_buf{VK_NULL_HANDLE};
  VkBufferCreateInfo stage_buffer_create_info{
      VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
  stage_buffer_create_info.size = size;
  stage_buffer_create_info.usage =
      VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
  VmaAllocationCreateInfo stage_create_info{};
  stage_create_info.usage = VMA_MEMORY_USAGE_AUTO;
  stage_create_info.flags =
      VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
      VMA_ALLOCATION_CREATE_MAPPED_BIT;
  VmaAllocation stage_alloc;
  VmaAllocationInfo stage_alloc_info;
  vmaCreateBuffer(rhi->m_vma_allocator, &stage_buffer_create_info,
                  &stage_create_info, &stage_buf, &stage_alloc,
                  &stage_alloc_info);

  memcpy(stage_alloc_info.pMappedData, data, stage_buffer_create_info.size);

  // transition image layout for cope.
  // transit for copy
  rhi->TransitImageLayout(dst, VK_IMAGE_LAYOUT_UNDEFINED,
                          VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, layer_count,
                          miplevels, aspect_mask_bits);

  VkCommandBuffer command_buffer = rhi->BeginSingleTimeCommands();

  VkBufferImageCopy region{};
  region.bufferOffset = 0;
  region.bufferRowLength = 0;
  region.bufferImageHeight = 0;
  region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  region.imageSubresource.mipLevel = miplevels;
  region.imageSubresource.baseArrayLayer = 0;
  region.imageSubresource.layerCount = layer_count;
  region.imageOffset = {0, 0, 0};
  region.imageExtent = {width, height, 1};
  vkCmdCopyBufferToImage(command_buffer, stage_buf, dst,
                         VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

  rhi->EndSingleTimeCommands(command_buffer);
  // transit for sampling.
  rhi->TransitImageLayout(dst, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                          VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, layer_count,
                          miplevels, aspect_mask_bits);

  // destory stage buffer.
  vmaDestroyBuffer(rhi->m_vma_allocator, stage_buf, stage_alloc);
}
}  // namespace ShaderStory
