#ifndef VK_UTILS_HPP
#define VK_UTILS_HPP

#include <vector>

#include "engine/runtime/render/vk_macros.h"
#include "third_party/shaderc/shaderc.hpp"
#include "third_party/vma/vk_mem_alloc.h"
namespace ShaderStory {
class VkUtil {
 public:
  static shaderc::Compiler compiler;

  static std::vector<u_int32_t> GetSpirvBinary(const std::string& file,
                                               shaderc_shader_kind shader_type);

  static VkShaderModule RuntimeCreateShaderModule(
      VkDevice device, const std::string& file,
      shaderc_shader_kind shader_type);

  static uint32_t FindMemoryType(
      const VkPhysicalDeviceMemoryProperties& properties, uint32_t type_filter,
      VkMemoryPropertyFlags flag);

  static void CreateBuffer(VkPhysicalDevice physical_device, VkDevice device,
                           VkDeviceSize size, VkBufferUsageFlags usage,
                           VkMemoryPropertyFlags properties, VkBuffer& buffer,
                           VkDeviceMemory& buffer_memory);

  static void CreateImage(VkPhysicalDevice physical_device, VkDevice device,
                          uint32_t image_width, uint32_t image_height,
                          VkFormat format, VkImageTiling image_tiling,
                          VkImageUsageFlags image_usage_flags,
                          VkMemoryPropertyFlags memory_property_flags,
                          VkImage& image, VkDeviceMemory& memory,
                          VkImageCreateFlags image_create_flags,
                          uint32_t array_layers, uint32_t miplevels);

  static VkImageView CreateImageView(VkDevice device, VkImage& image,
                                     VkFormat format,
                                     VkImageAspectFlags image_aspect_flags,
                                     VkImageViewType view_type,
                                     uint32_t layout_count, uint32_t miplevels);

  static void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer,
                         VkDeviceSize srcOffset, VkDeviceSize dstOffset,
                         VkDeviceSize size);
};

}  // namespace ShaderStory

#endif
