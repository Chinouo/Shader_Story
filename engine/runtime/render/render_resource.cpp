#include "engine/runtime/render/render_resource.hpp"

#include <iostream>

#include "engine/runtime/global/global.hpp"
#include "engine/runtime/render/rhi/vulkan/vk_rhi.hpp"

namespace ShaderStory {

void RenderResource::Initialize(std::shared_ptr<RHI::VKRHI> rhi) {
  m_rhi = rhi;

  m_perframe_ubo_manager.Initialize(rhi);
  m_perframe_sbo_manager.Initialize(rhi);

  m_ssao_resource_manager.Initialize(rhi);
  m_defered_resource_manager.Initialize(rhi);
  m_terrain_material_manager.Initialize(rhi);

  CreateSamplers();
  SetUpSunResources();

  std::vector<StaticMesh> mesh_1 =
      AssetsManager::LoadObjToStaticMeshes("assets/TEST.obj");

  for (const auto& mesh : mesh_1) {
    UploadStaticMesh(mesh);
  }
}

void RenderResource::Dispose() {
  auto allocator = m_rhi->m_vma_allocator;
  auto device = m_rhi->m_device;

  sun_resource_object.Dispose(allocator, device);

  m_perframe_ubo_manager.Destory(m_rhi);
  m_ssao_resource_manager.Destory(m_rhi);
  m_defered_resource_manager.Destory(m_rhi);
  m_terrain_material_manager.Destory(m_rhi);

  for (const auto& [k, v] : m_mesh_objects) {
    vmaDestroyBuffer(allocator, v.mesh_vert_buf, v.mesh_vert_alloc);
    vmaDestroyBuffer(allocator, v.mesh_indices_buf, v.mesh_indices_alloc);
  }

  {
    vkDestroySampler(device, m_sampler, nullptr);
    vkDestroySampler(device, m_neaest_sampler, nullptr);
  }
}

void RenderResource::UploadStaticMesh(const StaticMesh& mesh) {
  // create gpu only data.
  auto alloctor = m_rhi->m_vma_allocator;
  RenderStaticMeshObject static_mesh_obj{};
  static_mesh_obj.vert_count = mesh.vertices.size();
  static_mesh_obj.index_count = mesh.indices.size();
  // vert
  VkBufferCreateInfo vert_buf_create_info{VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
  vert_buf_create_info.size = mesh.GetVerticesSize();
  vert_buf_create_info.usage =
      VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
  VmaAllocationCreateInfo vert_alloc_create_info{};
  vert_alloc_create_info.priority = 1.f;
  vert_alloc_create_info.usage = VMA_MEMORY_USAGE_AUTO;
  vert_alloc_create_info.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;

  vmaCreateBuffer(alloctor, &vert_buf_create_info, &vert_alloc_create_info,
                  &static_mesh_obj.mesh_vert_buf,
                  &static_mesh_obj.mesh_vert_alloc, nullptr);

  StagingUpload(static_mesh_obj.mesh_vert_buf, vert_buf_create_info.size,
                mesh.vertices.data());

  // indices
  VkBufferCreateInfo indices_buf_create_info{
      VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
  indices_buf_create_info.size = mesh.GetIndicesSize();
  indices_buf_create_info.usage =
      VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
  VmaAllocationCreateInfo indices_alloc_create_info{};
  indices_alloc_create_info.priority = 1.f;
  indices_alloc_create_info.usage = VMA_MEMORY_USAGE_AUTO;
  indices_alloc_create_info.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;

  vmaCreateBuffer(alloctor, &indices_buf_create_info,
                  &indices_alloc_create_info, &static_mesh_obj.mesh_indices_buf,
                  &static_mesh_obj.mesh_indices_alloc, nullptr);

  StagingUpload(static_mesh_obj.mesh_indices_buf, indices_buf_create_info.size,
                mesh.indices.data());

  std::clog << mesh.name << " loaded.\n" << std::endl;
  // TODO: using GUID to alloc.
  //   ASSERT(!m_mesh_objects.count(mesh.name) &&
  //          "Current not support duplicated name mesh.");
  m_mesh_objects[mesh.name] = static_mesh_obj;

  std::cout << "vert size: " << vert_buf_create_info.size << '\n'
            << "indices size: " << indices_buf_create_info.size << std::endl;
}

void RenderResource::StagingUpload(VkBuffer dst, VkDeviceSize size,
                                   const void* data) {
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
  vmaCreateBuffer(m_rhi->m_vma_allocator, &stage_buffer_create_info,
                  &stage_create_info, &stage_buf, &stage_alloc,
                  &stage_alloc_info);

  memcpy(stage_alloc_info.pMappedData, data, stage_buffer_create_info.size);

  m_rhi->CopyBuffer(stage_buf, 0, dst, 0, size);

  // free staging buffer.
  vmaDestroyBuffer(m_rhi->m_vma_allocator, stage_buf, stage_alloc);
}

void RenderResource::UpdatePerFrameData(const SwapData& swap_data) {
  int cur_frame_idx = m_rhi->GetCurrentFrameIndex();
  const auto& perframe_data = swap_data.perframe_ubo_data;
  m_perframe_ubo_manager.UpdateCurrentFrameData(swap_data.perframe_ubo_data,
                                                cur_frame_idx);

  m_perframe_sbo_manager.UpdateCurrentFrameData(swap_data.perframe_sbo_data,
                                                cur_frame_idx);
}

void RenderResource::CreateSamplers() {
  // texture sampler
  VkSamplerCreateInfo sampler_create_info{
      VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO};
  sampler_create_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
  sampler_create_info.magFilter = VK_FILTER_NEAREST;  // minecraft is pixel!
  sampler_create_info.minFilter = VK_FILTER_NEAREST;
  sampler_create_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  sampler_create_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  sampler_create_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  sampler_create_info.anisotropyEnable = VK_TRUE;
  sampler_create_info.maxAnisotropy =
      m_rhi->m_pd_property.limits.maxSamplerAnisotropy;
  sampler_create_info.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
  sampler_create_info.unnormalizedCoordinates = VK_FALSE;
  sampler_create_info.compareEnable = VK_FALSE;
  sampler_create_info.compareOp = VK_COMPARE_OP_ALWAYS;
  sampler_create_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

  VK_CHECK(vkCreateSampler(m_rhi->m_device, &sampler_create_info, nullptr,
                           &m_sampler),
           "Create sampler failed.");

  // depth sampler
  VkSamplerCreateInfo shadowmap_sampler_create_info{
      VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO};
  shadowmap_sampler_create_info.magFilter = VK_FILTER_LINEAR;
  shadowmap_sampler_create_info.minFilter = VK_FILTER_LINEAR;
  shadowmap_sampler_create_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
  shadowmap_sampler_create_info.addressModeU =
      VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
  shadowmap_sampler_create_info.addressModeV =
      VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
  shadowmap_sampler_create_info.addressModeW =
      VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
  shadowmap_sampler_create_info.mipLodBias = 0.0f;
  shadowmap_sampler_create_info.maxAnisotropy = 1.0f;
  shadowmap_sampler_create_info.minLod = 0.0f;
  shadowmap_sampler_create_info.maxLod = 1.0f;
  shadowmap_sampler_create_info.anisotropyEnable = VK_FALSE;
  sampler_create_info.unnormalizedCoordinates = VK_FALSE;
  sampler_create_info.compareOp = VK_COMPARE_OP_ALWAYS;
  shadowmap_sampler_create_info.borderColor =
      VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;

  VK_CHECK(vkCreateSampler(m_rhi->m_device, &shadowmap_sampler_create_info,
                           nullptr, &sun_resource_object.shadowmap_sampler),
           "Create shadowmap sampler failed.");

  // nearest sampler
  VkSamplerCreateInfo nearest_sampler_info{
      VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO};
  nearest_sampler_info.magFilter = VK_FILTER_NEAREST;
  nearest_sampler_info.minFilter = VK_FILTER_NEAREST;
  nearest_sampler_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
  nearest_sampler_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
  nearest_sampler_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
  nearest_sampler_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
  nearest_sampler_info.mipLodBias = 0.0f;
  nearest_sampler_info.maxAnisotropy = 1.0f;
  nearest_sampler_info.minLod = 0.0f;
  nearest_sampler_info.maxLod = 1.0f;
  nearest_sampler_info.anisotropyEnable = VK_FALSE;
  nearest_sampler_info.unnormalizedCoordinates = VK_FALSE;
  nearest_sampler_info.compareOp = VK_COMPARE_OP_ALWAYS;

  VK_CHECK(vkCreateSampler(m_rhi->m_device, &nearest_sampler_info, nullptr,
                           &m_neaest_sampler),
           "Create shadowmap nearest failed.");
}

void RenderResource::SetUpSunResources() {
  VkFormat required_format = m_rhi->FindSupportFormat(
      {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT,
       VK_FORMAT_D24_UNORM_S8_UINT},
      VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);

  sun_resource_object.shadow_map_format = required_format;
  sun_resource_object.shadowmap_width = SunResourceObject::SHADOWMAP_RES;
  sun_resource_object.shadowmap_height = SunResourceObject::SHADOWMAP_RES;

  VmaAllocationCreateInfo alloc_info{};
  alloc_info.usage = VMA_MEMORY_USAGE_AUTO;
  alloc_info.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
  alloc_info.priority = 1.0f;

  VkImageCreateInfo image_info{VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
  image_info.imageType = VK_IMAGE_TYPE_2D;
  image_info.format = required_format;
  image_info.arrayLayers = SunResourceObject::SHADOWMAP_CNT;
  image_info.extent.width = sun_resource_object.shadowmap_width;
  image_info.extent.height = sun_resource_object.shadowmap_height;
  image_info.extent.depth = 1;
  image_info.mipLevels = 1;
  image_info.samples = VK_SAMPLE_COUNT_1_BIT;
  image_info.tiling = VK_IMAGE_TILING_OPTIMAL;
  image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  image_info.usage =
      VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

  VkImageViewCreateInfo view_info{VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
  view_info.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
  view_info.format = required_format;
  view_info.subresourceRange = {};
  view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
  view_info.subresourceRange.baseMipLevel = 0;
  view_info.subresourceRange.levelCount = 1;

  //  view for composite sampler

  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
    vmaCreateImage(m_rhi->m_vma_allocator, &image_info, &alloc_info,
                   &sun_resource_object.sun_depth[i].cascade_shadowmap_image,
                   &sun_resource_object.sun_depth[i].cascade_sm_alloc, nullptr);

    view_info.image = sun_resource_object.sun_depth[i].cascade_shadowmap_image;
    // single view ford depth read
    view_info.subresourceRange.layerCount = SunResourceObject::SHADOWMAP_CNT;
    view_info.subresourceRange.baseArrayLayer = 0;
    vkCreateImageView(
        m_rhi->m_device, &view_info, nullptr,
        &sun_resource_object.sun_depth[i].cascade_shadowmap_array_view);

    // multi views for depth write
    for (int j = 0; j < SunResourceObject::SHADOWMAP_CNT; ++j) {
      view_info.subresourceRange.baseArrayLayer = j;
      view_info.subresourceRange.layerCount = 1;

      // view for sun pass depth write.
      vkCreateImageView(
          m_rhi->m_device, &view_info, nullptr,
          &sun_resource_object.sun_depth[i].cascade_shadowmap_views[j]);
    }
  }
}

}  // namespace ShaderStory
