#ifndef SBO_HPP
#define SBO_HPP

#include <array>

#include "engine/core/math.hpp"
#include "engine/runtime/render/render_base.hpp"
#include "engine/runtime/render/render_swap_data.hpp"

namespace ShaderStory {

// a big chunk storage buffer
struct StorageBufferObject {
  static const u_int32_t min_algiment{256};

  VkBuffer perframe_data_buffer{VK_NULL_HANDLE};
  VmaAllocation perframe_data_alloc{VK_NULL_HANDLE};
  VmaAllocationInfo perframe_data_alloc_info;
  void* mapped_memory{nullptr};
};

class StorageBufferObjectManager : public RenderResourceBase {
 public:
  StorageBufferObjectManager();
  ~StorageBufferObjectManager();

  void Initialize(std::shared_ptr<RHI::VKRHI> rhi) override;
  void Destory(std::shared_ptr<RHI::VKRHI> rhi) override;

  void UpdateCurrentFrameData(const PerframeDataSBO& data,
                              int current_frame_idx);

  VkDescriptorBufferInfo GetDespBufInfo() const;
  VkWriteDescriptorSet GetDespWrite() const;

  constexpr u_int32_t GetPerframeSBODynamicOffset() const {
    const size_t sz = sizeof(PerframeDataSBO);
    return (m_perframe_sbo.min_algiment + sz - 1) & ~(sz - 1);
  }

 private:
  StorageBufferObject m_perframe_sbo;
};

};  // namespace ShaderStory

#endif