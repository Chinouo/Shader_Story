#include "engine/runtime/render/rhi/vulkan/vk_utils.hpp"

#include <fstream>

#include "engine/common/macros.h"
namespace ShaderStory {
shaderc::Compiler VkUtil::compiler;

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

  ASSERT(result.GetErrorMessage().empty());

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

void VkUtil::CreateBuffer(VkPhysicalDevice physical_device, VkDevice device,
                          VkDeviceSize size, VkBufferUsageFlags usage,
                          VkMemoryPropertyFlags properties, VkBuffer& buffer,
                          VkDeviceMemory& buffer_memory) {}

void VkUtil::CreateImage(VkPhysicalDevice physical_device, VkDevice device,
                         uint32_t image_width, uint32_t image_height,
                         VkFormat format, VkImageTiling image_tiling,
                         VkImageUsageFlags image_usage_flags,
                         VkMemoryPropertyFlags memory_property_flags,
                         VkImage& image, VkDeviceMemory& memory,
                         VkImageCreateFlags image_create_flags,
                         uint32_t array_layers, uint32_t miplevels) {
  VkImageCreateInfo info{VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
  info.flags = image_create_flags;
  info.imageType = VK_IMAGE_TYPE_2D;
  info.extent.width = image_width;
  info.extent.height = image_height;
  info.extent.depth = 1;
  info.mipLevels = miplevels;
  info.arrayLayers = array_layers;
  info.format = format;
  info.tiling = image_tiling;
  info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  info.usage = image_usage_flags;
  info.samples = VK_SAMPLE_COUNT_1_BIT;
  info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
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

}  // namespace ShaderStory
