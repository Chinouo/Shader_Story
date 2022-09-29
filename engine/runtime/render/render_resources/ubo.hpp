#ifndef UBO_HPP
#define UBO_HPP

#include <array>

#include "engine/runtime/render/render_base.hpp"
#include "engine/runtime/render/render_swap_data.hpp"

namespace ShaderStory {

struct PerframeUniformBufferObject {
  // spec allowed max aligment of uniform buffer in bytes.
  static const u_int32_t min_algiment{256};

  VkBuffer perframe_data_buffer{VK_NULL_HANDLE};
  VmaAllocation perframe_data_alloc{VK_NULL_HANDLE};
  VmaAllocationInfo perframe_data_alloc_info;
  void* mapped_memory;
};

/// A big block uniform buffer data, we store perframe data in it,
/// such as camera matrix, light position ...
class UniformObjectManager : public RenderResourceBase {
 public:
  UniformObjectManager();
  ~UniformObjectManager();

  void Initialize(std::shared_ptr<RHI::VKRHI> rhi) override;
  void Destory(std::shared_ptr<RHI::VKRHI> rhi) override;

  void UpdateCurrentFrameData(const PerframeDataUBO& data,
                              int current_frame_idx);

  // util
  const PerframeUniformBufferObject& GetPerframeUBO() const {
    return m_perframe_ubo;
  }

  VkDescriptorBufferInfo GetDespBufInfo() const;
  VkWriteDescriptorSet GetDespWrite() const;

  constexpr u_int32_t GetPerframeUBODynamicOffset() const {
    const size_t sz = sizeof(PerframeDataUBO);
    return (m_perframe_ubo.min_algiment + sz - 1) & ~(sz - 1);
  }

 private:
 private:
  PerframeUniformBufferObject m_perframe_ubo;
};
}  // namespace ShaderStory

#endif