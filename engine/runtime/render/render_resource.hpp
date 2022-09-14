#ifndef RENDER_RESOURCE_HPP
#define RENDER_RESOURCE_HPP
#include <vulkan/vulkan.h>

#include <unordered_map>

#include "engine/common/macros.h"
#include "engine/component/camera.hpp"
#include "engine/runtime/io/assets_manager.hpp"
#include "engine/runtime/render/render_swap_data.hpp"
#include "engine/runtime/render/rhi/vulkan/vk_rhi.hpp"
#include "engine/runtime/render/shader_types.hpp"
#include "third_party/vma/vk_mem_alloc.h"

namespace ShaderStory {

/// perform like a ring buffer.
struct PerframeDataBufferObject {
  VkBuffer perframe_data_buffer{VK_NULL_HANDLE};
  VmaAllocation perframe_data_alloc{VK_NULL_HANDLE};
  VmaAllocationInfo perframe_data_alloc_info;

  PerframeData perframe_data;
  void* mapped_memory;

  // spec allowed max aligment of uniform buffer in bytes.
  u_int32_t min_algiment{256};

  void SetData(const PerframeData& data, int index) {
    void* dst_with_offset = (char*)mapped_memory + index * min_algiment;
    memcpy(dst_with_offset, &data, sizeof(PerframeData));
  };
};

/// basic simple mesh data.
struct RenderStaticMeshObject {
  VkBuffer mesh_vert_buf;
  VmaAllocation mesh_vert_alloc;
  u_int32_t vert_count;

  VkBuffer mesh_indices_buf;
  VmaAllocation mesh_indices_alloc;
  u_int32_t index_count;
};

/// GPU Data detail, there is a saying: struct store arrays inside may better
/// than store a  array struct.
struct RenderMesh {
  uint32_t mesh_vertex_count;

  // posiiton
  VkBuffer mesh_vertices_position_buffer;
  VmaAllocation mesh_vertices_position_vma_alloc;

  // normal
  VkBuffer mesh_vertices_normal_buffer;
  VmaAllocation mesh_vertices_normal_vma_alloc;

  // tangent
  VkBuffer mesh_vertices_tangent_buffer;
  VmaAllocation mesh_vertices_tangent_vma_alloc;

  // texcoords
  VkBuffer mesh_vertices_texcoords_buffer;
  VmaAllocation mesh_vertices_texcoords_vma_alloc;

  uint32_t mesh_index_count;
  VkBuffer mesh_index_buffer;
  VmaAllocation mesh_index_buffer_vma_alloc;
};

/// Our terrain texture.
struct RenderTerrainTextureObject {
  VkImage texture_image;
  VkImageView texture_image_view;
  VmaAllocation texture_alloc;

  void Dispose(VmaAllocator allocator) {
    vmaDestroyImage(allocator, texture_image, texture_alloc);
  }
};

/// Sun (direction light without position)
struct SunResourceObject {
  VkImage sun_shadowmap_image{VK_NULL_HANDLE};
  VkImageView sun_shadowmap_image_view{VK_NULL_HANDLE};
  VmaAllocation sun_shadowmap_alloc{VK_NULL_HANDLE};

  VkFormat shadow_map_format{VK_FORMAT_UNDEFINED};
  VkSampler shadowmap_sampler{VK_NULL_HANDLE};

  u_int32_t shadowmap_width;
  u_int32_t shadowmap_height;

  void Dispose(VmaAllocator allocator, VkDevice device) {
    vmaDestroyImage(allocator, sun_shadowmap_image, sun_shadowmap_alloc);
    vkDestroyImageView(device, sun_shadowmap_image_view, nullptr);
    vkDestroySampler(device, shadowmap_sampler, nullptr);
  }
};

/// manager GPU data...
/// singleton
class RenderResource final {
 public:
  RenderResource() = default;
  ~RenderResource() = default;

  void Initialize(std::shared_ptr<RHI::VKRHI> rhi);
  void Dispose();

  // function below are used by our logic pass, see each xxxpass for detail.
 public:
  const std::unordered_map<std::string, RenderStaticMeshObject>&
  GetMeshesObject() {
    return m_mesh_objects;
  }

  const PerframeDataBufferObject& GetPerframeDataObject() const {
    return perframe_data_obj;
  }

  const RenderTerrainTextureObject& GetTerrainTextureObject() const {
    return m_terrain_texture_object;
  }

  const SunResourceObject& GetSunResourceObject() const {
    return sun_resource_object;
  }

  VkSampler GetTerrainSampler() const { return m_sampler; }

 public:
  // update data.
  void UpdatePerFrameData(const SwapData& swap_data);

  // load static mesh data from CPU to GPU.
  void UploadMeshData(RenderMeshVertexBasic&);

  // update obj texture 2D for sampling.
  void UploadTerrainTexture(Texture2D& info);

  // perframe data, camera etc.
  void CreatePerFrameData();

  void UploadStaticMesh(const StaticMesh&);

 private:
  void StagingUpload(VkBuffer dst, VkDeviceSize size, const void* data);

  void StagingUploadImage(VkImage dst, u_int32_t width, u_int32_t height,
                          VkDeviceSize size, uint32_t layer_count,
                          uint32_t miplevels,
                          VkImageAspectFlags aspect_mask_bits,
                          const void* data);

  void CreateSamplers();

  void MakeSun();

 private:
  /// perframe data obj
  PerframeDataBufferObject perframe_data_obj;

  /// store all loaded mesh, data located in GPU.
  std::unordered_map<std::string, RenderStaticMeshObject> m_mesh_objects;

  /// loaded terrain texture for sample
  RenderTerrainTextureObject m_terrain_texture_object;

  ///  sampler, can use for terrain sampler.
  VkSampler m_sampler{VK_NULL_HANDLE};

  /// sun shadowmap resource
  SunResourceObject sun_resource_object;

 private:
  std::shared_ptr<RHI::VKRHI> m_rhi;
  DISALLOW_COPY_ASSIGN_AND_MOVE(RenderResource);
};

}  // namespace ShaderStory

#endif
