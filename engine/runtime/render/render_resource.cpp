#include "engine/runtime/render/render_resource.hpp"

#include <iostream>

#include "engine/runtime/global/global.hpp"
#include "engine/runtime/render/rhi/vulkan/vk_rhi.hpp"

namespace ShaderStory {

void RenderResource::Initialize(std::shared_ptr<RHI::VKRHI> rhi) {
  m_rhi = rhi;

  CreateSamplers();
  MakeSun();
  Texture2D big_texture =
      AssetsManager::LoadTextureFile("assets/flat-RGBA.png");

  UploadTerrainTexture(big_texture);
  big_texture.Dispose();

  std::vector<StaticMesh> mesh_1 =
      AssetsManager::LoadObjToStaticMeshes("assets/flat.obj");

  for (const auto& mesh : mesh_1) {
    UploadStaticMesh(mesh);
  }
  CreatePerFrameData();
}

void RenderResource::Dispose() {
  auto allocator = m_rhi->m_vma_allocator;
  auto device = m_rhi->m_device;

  sun_resource_object.Dispose(allocator, device);

  vmaUnmapMemory(allocator, perframe_data_obj.perframe_data_alloc);
  vmaDestroyBuffer(allocator, perframe_data_obj.perframe_data_buffer,
                   perframe_data_obj.perframe_data_alloc);

  for (const auto& [k, v] : m_mesh_objects) {
    vmaDestroyBuffer(allocator, v.mesh_vert_buf, v.mesh_vert_alloc);
    vmaDestroyBuffer(allocator, v.mesh_indices_buf, v.mesh_indices_alloc);
  }

  { vkDestroySampler(device, m_sampler, nullptr); }

  // free terrain texture
  {
    vmaDestroyImage(allocator, m_terrain_texture_object.texture_image,
                    m_terrain_texture_object.texture_alloc);
    vkDestroyImageView(device, m_terrain_texture_object.texture_image_view,
                       nullptr);
  }
}

void RenderResource::CreatePerFrameData() {
  VkBufferCreateInfo buf_create_info{VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
  buf_create_info.size = perframe_data_obj.min_algiment * MAX_FRAMES_IN_FLIGHT;
  buf_create_info.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;

  VmaAllocationCreateInfo alloc_create_info{};
  alloc_create_info.usage = VMA_MEMORY_USAGE_AUTO;
  alloc_create_info.priority = 1.0f;
  alloc_create_info.flags =
      VMA_ALLOCATION_CREATE_MAPPED_BIT |
      VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
      VMA_ALLOCATION_CREATE_HOST_ACCESS_ALLOW_TRANSFER_INSTEAD_BIT;

  vmaCreateBufferWithAlignment(
      m_rhi->m_vma_allocator, &buf_create_info, &alloc_create_info,
      perframe_data_obj.min_algiment, &perframe_data_obj.perframe_data_buffer,
      &perframe_data_obj.perframe_data_alloc,
      &perframe_data_obj.perframe_data_alloc_info);

  VkMemoryPropertyFlags memPropFlags;
  vmaGetAllocationMemoryProperties(m_rhi->m_vma_allocator,
                                   perframe_data_obj.perframe_data_alloc,
                                   &memPropFlags);

  ASSERT(memPropFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT &&
         "Allocate Perframe Uniform Buffer Failed.");

  vmaMapMemory(m_rhi->m_vma_allocator, perframe_data_obj.perframe_data_alloc,
               &perframe_data_obj.mapped_memory);
  ASSERT(perframe_data_obj.perframe_data_alloc_info.pMappedData ==
         perframe_data_obj.mapped_memory);
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
  // TODO: using GUID to alloc.
  ASSERT(!m_mesh_objects.count(mesh.name) &&
         "Current not support duplicated name mesh.");
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
  const PerframeData& perframe_data = swap_data.perframe_data;

  perframe_data_obj.SetData(perframe_data, cur_frame_idx);
}

void RenderResource::StagingUploadImage(VkImage dst, u_int32_t width,
                                        u_int32_t height, VkDeviceSize size,
                                        uint32_t layer_count,
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
  vmaCreateBuffer(m_rhi->m_vma_allocator, &stage_buffer_create_info,
                  &stage_create_info, &stage_buf, &stage_alloc,
                  &stage_alloc_info);

  memcpy(stage_alloc_info.pMappedData, data, stage_buffer_create_info.size);

  // transition image layout for cope.
  // transit for copy
  m_rhi->TransitImageLayout(dst, VK_IMAGE_LAYOUT_UNDEFINED,
                            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, layer_count,
                            miplevels, aspect_mask_bits);

  VkCommandBuffer command_buffer = m_rhi->BeginSingleTimeCommands();

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

  m_rhi->EndSingleTimeCommands(command_buffer);
  // transit for sampling.
  m_rhi->TransitImageLayout(dst, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                            layer_count, miplevels, aspect_mask_bits);

  // destory stage buffer.
  vmaDestroyBuffer(m_rhi->m_vma_allocator, stage_buf, stage_alloc);
}

void RenderResource::UploadTerrainTexture(Texture2D& info) {
  RenderTerrainTextureObject& object = m_terrain_texture_object;
  VmaAllocationCreateInfo alloc_info{};
  alloc_info.usage = VMA_MEMORY_USAGE_AUTO;
  alloc_info.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
  alloc_info.priority = 1.0f;

  VkImageCreateInfo image_create_info = info.GetDefaultImageCreateInfo();
  image_create_info.usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;

  vmaCreateImage(m_rhi->m_vma_allocator, &image_create_info, &alloc_info,
                 &object.texture_image, &object.texture_alloc, nullptr);

  VkImageViewCreateInfo view_create_info{
      VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
  view_create_info.image = object.texture_image;
  view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
  view_create_info.format = VK_FORMAT_R8G8B8A8_SRGB;
  view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  view_create_info.subresourceRange.baseMipLevel = 0;
  view_create_info.subresourceRange.levelCount = 1;
  view_create_info.subresourceRange.baseArrayLayer = 0;
  view_create_info.subresourceRange.layerCount = 1;

  vkCreateImageView(m_rhi->m_device, &view_create_info, nullptr,
                    &object.texture_image_view);

  StagingUploadImage(object.texture_image, info.width, info.height, info.size,
                     1, 0, VK_IMAGE_ASPECT_COLOR_BIT, info.data);
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
      VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;

  VK_CHECK(vkCreateSampler(m_rhi->m_device, &shadowmap_sampler_create_info,
                           nullptr, &sun_resource_object.shadowmap_sampler),
           "Create shadowmap sampler failed.");
}

void RenderResource::MakeSun() {
  VkFormat required_format = m_rhi->FindSupportFormat(
      {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT,
       VK_FORMAT_D24_UNORM_S8_UINT},
      VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
  sun_resource_object.shadow_map_format = required_format;
  sun_resource_object.shadowmap_width = 2048;
  sun_resource_object.shadowmap_height = 2048;
  //   sun_resource_object.shadowmap_width = m_rhi->m_swapchain_extent.width;
  //   sun_resource_object.shadowmap_height = m_rhi->m_swapchain_extent.height;

  VmaAllocationCreateInfo alloc_info{};
  alloc_info.usage = VMA_MEMORY_USAGE_AUTO;
  alloc_info.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
  alloc_info.priority = 1.0f;

  VkImageCreateInfo image_create_info{VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
  image_create_info.imageType = VK_IMAGE_TYPE_2D;
  image_create_info.format = required_format;
  image_create_info.arrayLayers = 1;
  image_create_info.extent.width = sun_resource_object.shadowmap_width;
  image_create_info.extent.height = sun_resource_object.shadowmap_height;
  image_create_info.extent.depth = 1;
  image_create_info.mipLevels = 1;
  image_create_info.arrayLayers = 1;
  image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
  image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
  image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  image_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  image_create_info.usage =
      VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

  vmaCreateImage(m_rhi->m_vma_allocator, &image_create_info, &alloc_info,
                 &sun_resource_object.sun_shadowmap_image,
                 &sun_resource_object.sun_shadowmap_alloc, nullptr);

  VkImageViewCreateInfo view_create_info{
      VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
  view_create_info.image = sun_resource_object.sun_shadowmap_image;
  view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
  view_create_info.format = required_format;
  view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
  view_create_info.subresourceRange.baseMipLevel = 0;
  view_create_info.subresourceRange.levelCount = 1;
  view_create_info.subresourceRange.baseArrayLayer = 0;
  view_create_info.subresourceRange.layerCount = 1;

  vkCreateImageView(m_rhi->m_device, &view_create_info, nullptr,
                    &sun_resource_object.sun_shadowmap_image_view);
}

}  // namespace ShaderStory
