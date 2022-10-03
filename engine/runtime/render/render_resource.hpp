#ifndef RENDER_RESOURCE_HPP
#define RENDER_RESOURCE_HPP
#include <vulkan/vulkan.h>

#include <unordered_map>

#include "engine/common/macros.h"
#include "engine/component/camera.hpp"
#include "engine/runtime/io/assets_manager.hpp"
#include "engine/runtime/render/render_resources/defered_resource.hpp"
#include "engine/runtime/render/render_resources/materials.hpp"
#include "engine/runtime/render/render_resources/ssao_resource.hpp"
#include "engine/runtime/render/render_resources/ubo.hpp"
#include "engine/runtime/render/render_swap_data.hpp"
#include "engine/runtime/render/rhi/vulkan/vk_rhi.hpp"
#include "engine/runtime/render/shader_types.hpp"
#include "third_party/vma/vk_mem_alloc.h"

namespace ShaderStory {

/// dynamic data, using readonly storage buffer, similar to uniform.
struct PerframeStorageBufferObject {
  PerframeStorageBufferData data;
  void* mapped_mem;
  const u_int32_t mem_align{256};

  VkBuffer buf{VK_NULL_HANDLE};
  VmaAllocation alloc{VK_NULL_HANDLE};
  VmaAllocationInfo alloc_info;

  constexpr size_t GetOffset(size_t sz) {
    return (mem_align + sz - 1) & ~(sz - 1);
  }

  void SetData(const PerframeStorageBufferData& data, int index) {
    size_t sz = sizeof(PerframeStorageBufferData);
    void* dst_with_offset = (char*)(mapped_mem) + index * GetOffset(sz);
    memcpy(dst_with_offset, &data, sz);
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

struct SunDepthObject {
  VkImage cascade_shadowmap_image{VK_NULL_HANDLE};
  VmaAllocation cascade_sm_alloc{VK_NULL_HANDLE};
  // composite pass:sampler2DArray
  VkImageView cascade_shadowmap_array_view{VK_NULL_HANDLE};
  // sun pass: desp bind to write
  std::array<VkImageView, 3> cascade_shadowmap_views;
};

/// Sun (direction light without position)
struct SunResourceObject {
  static const int SHADOWMAP_RES = 2048;
  static const int SHADOWMAP_CNT = 3;

  std::array<SunDepthObject, MAX_FRAMES_IN_FLIGHT> sun_depth;

  VkFormat shadow_map_format{VK_FORMAT_UNDEFINED};
  VkSampler shadowmap_sampler{VK_NULL_HANDLE};
  u_int32_t shadowmap_width;
  u_int32_t shadowmap_height;

  void Dispose(VmaAllocator allocator, VkDevice device) {
    // vmaDestroyImage(allocator, cascade_shadowmap_image,
    //                 cascade_shadowmap_alloc);
    // vkDestroyImageView(device, cascade_shadowmap_view, nullptr);
    // vkDestroySampler(device, shadowmap_sampler, nullptr);
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

  const UniformObjectManager& GetPerframeUBOManager() const {
    return m_perframe_ubo_manager;
  }

  const SSAOResourceManager& GetSSAOResourceManager() const {
    return m_ssao_resource_manager;
  }

  const DeferedResourceManager& GetDeferedResourceManager() const {
    return m_defered_resource_manager;
  }

  const MaterialManager& GetTerrainMaterialManager() const {
    return m_terrain_material_manager;
  }

  // const RenderTerrainTextureObject& GetTerrainTextureObject() const {
  //   return m_terrain_texture_object;
  // }

  const SunResourceObject& GetSunResourceObject() const {
    return sun_resource_object;
  }

  /// @deprecated
  VkSampler GetTerrainSampler() const { return m_sampler; }

  VkSampler GetDefaultNearestSampler() const { return m_neaest_sampler; }

 public:
  // update data.
  void UpdatePerFrameData(const SwapData& swap_data);

  // load static mesh data from CPU to GPU.
  void UploadMeshData(RenderMeshVertexBasic&);

  // update obj texture 2D for sampling.
  void UploadTerrainTexture(Texture2D& info);

  // perframe data, camera etc.
  void CreatePerFrameData();

  void CreatePerFrameStorageBuffer();

  void CreateDeferedObject();

  void UploadStaticMesh(const StaticMesh&);

 private:
  void StagingUpload(VkBuffer dst, VkDeviceSize size, const void* data);

  void StagingUploadImage(VkImage dst, u_int32_t width, u_int32_t height,
                          VkDeviceSize size, uint32_t layer_count,
                          uint32_t miplevels,
                          VkImageAspectFlags aspect_mask_bits,
                          const void* data);

  void CreateSamplers();

  void SetUpSunResources();

 private:
  /// perframe data obj
  UniformObjectManager m_perframe_ubo_manager;

  SSAOResourceManager m_ssao_resource_manager;

  DeferedResourceManager m_defered_resource_manager;

  PerframeStorageBufferObject perframe_storage_obj;
  /// store all loaded mesh, data located in GPU.
  std::unordered_map<std::string, RenderStaticMeshObject> m_mesh_objects;

  MaterialManager m_terrain_material_manager;

  ///  sampler, can use for terrain sampler.
  VkSampler m_sampler{VK_NULL_HANDLE};

  // TODO:
  /// defalut samplers.
  VkSampler m_liner_sampler{VK_NULL_HANDLE};
  VkSampler m_neaest_sampler{VK_NULL_HANDLE};

  /// sun shadowmap resource
  SunResourceObject sun_resource_object;

  // GBufferResources m_g_buffer_resources;

 private:
  std::shared_ptr<RHI::VKRHI> m_rhi;
  DISALLOW_COPY_ASSIGN_AND_MOVE(RenderResource);
};

}  // namespace ShaderStory

#endif
