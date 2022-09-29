#ifndef VK_UTILS_HPP
#define VK_UTILS_HPP

#include <vector>

#include "engine/runtime/render/rhi/vulkan/vk_rhi.hpp"
#include "third_party/shaderc/shaderc.hpp"
#include "third_party/vma/vk_mem_alloc.h"

namespace ShaderStory {

class VkUtil {
 public:
  static shaderc::Compiler compiler;
  static shaderc::CompileOptions compile_op;

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

  static VkImageView CreateImageView(VkDevice device, VkImage& image,
                                     VkFormat format,
                                     VkImageAspectFlags image_aspect_flags,
                                     VkImageViewType view_type,
                                     uint32_t layout_count, uint32_t miplevels);

  static void TransitionImageLayout(VkImage image, VkFormat format,
                                    VkImageLayout old_layout,
                                    VkImageLayout new_layout);

  static void StageUploadBuffer(std::shared_ptr<RHI::VKRHI> rhi, VkBuffer dst,
                                VkDeviceSize size, const void* data);

  static void StageUploadImage(std::shared_ptr<RHI::VKRHI> rhi, VkImage dst,
                               u_int32_t width, u_int32_t height,
                               VkDeviceSize size, uint32_t layer_count,
                               uint32_t miplevels,
                               VkImageAspectFlags aspect_mask_bits,
                               const void* data);
};

// TODO: imp include
class SettingIncluder : public shaderc::CompileOptions::IncluderInterface {
 public:
  SettingIncluder() = default;
  ~SettingIncluder() = default;

  shaderc_include_result* GetInclude(const char* requested_source,
                                     shaderc_include_type type,
                                     const char* requesting_source,
                                     size_t include_depth) override;

  void ReleaseInclude(shaderc_include_result* data) override;
};

}  // namespace ShaderStory

#endif
