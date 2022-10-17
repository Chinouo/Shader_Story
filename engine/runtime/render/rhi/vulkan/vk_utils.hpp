#ifndef VK_UTILS_HPP
#define VK_UTILS_HPP

#include <fstream>
#include <unordered_map>
#include <vector>

#include "engine/runtime/render/rhi/vulkan/vk_rhi.hpp"
#include "third_party/shaderc/shaderc.hpp"
#include "third_party/vma/vk_mem_alloc.h"

namespace ShaderStory {
/// may need move construct.
class GLSLInclude : public shaderc::CompileOptions::IncluderInterface {
 public:
  GLSLInclude(const std::string path_prefix,
              const std::vector<std::string>& include_files);
  ~GLSLInclude();

  shaderc_include_result* GetInclude(const char* requested_source,
                                     shaderc_include_type type,
                                     const char* requesting_source,
                                     size_t include_depth) override;

  // Handles shaderc_include_result_release_fn callbacks.
  virtual void ReleaseInclude(shaderc_include_result* data) override;

  const std::string m_path_prefix;

  std::unordered_map<std::string, std::vector<char>> m_contents;
};

inline GLSLInclude::GLSLInclude(const std::string path_prefix,
                                const std::vector<std::string>& include_files) {
  // make sure non-duplicated.
  for (const auto& include_file : include_files) {
    const std::string file = path_prefix + include_file;
    std::fstream in(file, std::ios::in);

    ASSERT(in.is_open() && "Failed to open file");
    in.seekg(0, std::ios::end);  // 从末尾开始计算偏移量
    std::streampos size = in.tellg();
    in.seekg(0, std::ios::beg);  // 从起始位置开始计算偏移量

    std::vector<char> buf(size);

    in.read(buf.data(), size);  // 读取数据
    in.close();

    // store full absolute path.
    m_contents.try_emplace(include_file, buf);
  }
}

inline GLSLInclude::~GLSLInclude() {}

/// eg: a.h need b.h, requested_source is b.h, requesting_source is a.h.
inline shaderc_include_result* GLSLInclude::GetInclude(
    const char* requested_source, shaderc_include_type type,
    const char* requesting_source, size_t include_depth) {
  shaderc_include_result* result = new shaderc_include_result;

  std::string include_source(requested_source);
  include_source = m_path_prefix + include_source;
  assert(m_contents.count(include_source));

  const auto it = m_contents.find(include_source);

  result->content = it->second.data();
  result->content_length = it->second.size();

  result->source_name = it->first.data();
  result->source_name_length = it->first.size();
  result->user_data = nullptr;

  return result;
}

inline void GLSLInclude::ReleaseInclude(shaderc_include_result* data) {
  delete data;
}

class VkUtil {
 public:
  static shaderc::Compiler compiler;

  static std::vector<u_int32_t> GetSpirvBinary(const std::string& file,
                                               shaderc_shader_kind shader_type);

  static VkShaderModule RuntimeCreateShaderModule(
      VkDevice device, const std::string& file,
      shaderc_shader_kind shader_type);

  static VkShaderModule RuntimeCreateShaderModule(
      const VkDevice device, const std::string& src_file,
      const std::string& path_prefix,
      const std::vector<std::string>& include_files,
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

  static void StageUploadImage(const std::shared_ptr<RHI::VKRHI>& rhi,
                               VkImage dst, u_int32_t width, u_int32_t height,
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
