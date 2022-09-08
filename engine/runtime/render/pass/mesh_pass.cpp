#include "engine/runtime/render/pass/mesh_pass.hpp"

#include <iostream>

#include "engine/common/macros.h"
#include "engine/runtime/render/rhi/vulkan/vk_utils.hpp"

namespace ShaderStory {

MeshPass::MeshPass() { std::cout << "MeshPass create.\n"; }

MeshPass::~MeshPass() {
  Dispose();
  std::cout << "MeshPass destory.\n";
}

void MeshPass::Dispose() {
  auto device = m_rhi->m_device;

  for (auto& depth_obj : m_depth_objects) {
    depth_obj.Dispose(m_rhi->m_vma_allocator, device);
  }

  if (m_mesh_desp_set_layout)
    vkDestroyDescriptorSetLayout(device, m_mesh_desp_set_layout, nullptr);
  if (m_mesh_desp_set)
    vkFreeDescriptorSets(device, m_rhi->m_descriptor_pool, 1, &m_mesh_desp_set);
  if (m_mesh_pipeline) vkDestroyPipeline(device, m_mesh_pipeline, nullptr);
  if (m_mesh_pipeline_layout)
    vkDestroyPipelineLayout(device, m_mesh_pipeline_layout, nullptr);
  if (m_mesh_pass) vkDestroyRenderPass(device, m_mesh_pass, nullptr);
  for (auto fb : m_framebuffers) {
    if (fb) vkDestroyFramebuffer(device, fb, nullptr);
  }
}

void MeshPass::Initialize() {
  CreateDepthResources();
  CreateVkRenderPass();
  CreateDesciptorSetLayout();
  CreateVkRenderPipeline();
  CreateDesciptorSet();
  CreateFrameBuffers();
}

void MeshPass::RunPass() {
  VkCommandBuffer command_buffer = m_rhi->GetCurrentCommandBuffer();
  VkRenderPassBeginInfo renderPassInfo{};
  renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  renderPassInfo.renderPass = m_mesh_pass;
  renderPassInfo.framebuffer =
      m_framebuffers[m_rhi->m_current_swapchain_image_index];
  renderPassInfo.renderArea.offset = {0, 0};
  renderPassInfo.renderArea.extent = m_rhi->m_swapchain_extent;

  std::array<VkClearValue, 2> clear_vals;
  clear_vals[0].color = {{0.3f, 0.4f, 0.5f, 1.0f}};
  clear_vals[1].depthStencil = {1.f, 0};
  renderPassInfo.clearValueCount = clear_vals.size();
  renderPassInfo.pClearValues = clear_vals.data();

  vkCmdBeginRenderPass(command_buffer, &renderPassInfo,
                       VK_SUBPASS_CONTENTS_INLINE);

  vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                    m_mesh_pipeline);
  u_int32_t cur_frame_idx = m_rhi->GetCurrentFrameIndex();
  u_int32_t algiment = m_resources->GetPerframeDataObject().min_algiment;
  u_int32_t dy_offsets = cur_frame_idx * algiment;

  vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                          m_mesh_pipeline_layout, 0, 1, &m_mesh_desp_set, 1,
                          &dy_offsets);

  VkDeviceSize offsets[] = {0};
  for (const auto& [k, v] : m_resources->GetMeshesObject()) {
    vkCmdBindVertexBuffers(command_buffer, 0, 1, &v.mesh_vert_buf, offsets);
    vkCmdBindIndexBuffer(command_buffer, v.mesh_indices_buf, 0,
                         VK_INDEX_TYPE_UINT32);
    vkCmdDrawIndexed(command_buffer, v.index_count, 1, 0, 0, 0);
  }
}

VkFormat MeshPass::PickDepthFormat() {
  return m_rhi->FindSupportFormat(
      {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT,
       VK_FORMAT_D24_UNORM_S8_UINT},
      VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
}

void MeshPass::CreateDepthResources() {
  VkImageCreateInfo image_create_info{VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
  image_create_info.format = PickDepthFormat();
  image_create_info.imageType = VK_IMAGE_TYPE_2D;
  image_create_info.extent.width = m_rhi->m_swapchain_extent.width;
  image_create_info.extent.height = m_rhi->m_swapchain_extent.height;
  image_create_info.extent.depth = 1;
  image_create_info.mipLevels = 1;
  image_create_info.arrayLayers = 1;
  image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
  image_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  image_create_info.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
  image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
  image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

  VkImageViewCreateInfo view_create_info{
      VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
  view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
  view_create_info.format = image_create_info.format;
  view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
  view_create_info.subresourceRange.baseMipLevel = 0;
  view_create_info.subresourceRange.levelCount = 1;
  view_create_info.subresourceRange.baseArrayLayer = 0;
  view_create_info.subresourceRange.layerCount = 1;

  VmaAllocationCreateInfo alloc_info{};
  alloc_info.usage = VMA_MEMORY_USAGE_AUTO;
  alloc_info.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
  alloc_info.priority = 1.0f;

  m_depth_objects.resize(MAX_FRAMES_IN_FLIGHT);
  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
    vmaCreateImage(m_rhi->m_vma_allocator, &image_create_info, &alloc_info,
                   &m_depth_objects[i].depth_image,
                   &m_depth_objects[i].depth_alloc, nullptr);

    view_create_info.image = m_depth_objects[i].depth_image;
    vkCreateImageView(m_rhi->m_device, &view_create_info, nullptr,
                      &m_depth_objects[i].depth_image_view);
  }
}

void MeshPass::CreateVkRenderPass() {
  std::array<VkAttachmentDescription, 2> attachments;
  // color attach
  attachments[0].format = m_rhi->m_swapchain_format;
  attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
  attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  attachments[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

  // depth attach
  attachments[1].format = PickDepthFormat();
  attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
  attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  attachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;

  std::array<VkAttachmentReference, 2> attachment_refs;

  // color attachment ref
  attachment_refs[0].attachment = 0;
  attachment_refs[0].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

  // depth attachment ref
  attachment_refs[1].attachment = 1;
  attachment_refs[1].layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

  VkSubpassDescription subpass{};
  subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
  subpass.colorAttachmentCount = 1;
  subpass.pColorAttachments = &attachment_refs[0];
  subpass.inputAttachmentCount = 0;
  subpass.pDepthStencilAttachment = &attachment_refs[1];

  std::array<VkSubpassDependency, 1> dependencies;
  dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
  dependencies[0].dstSubpass = 0;
  dependencies[0].srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
  dependencies[0].dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
  dependencies[0].srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
  dependencies[0].dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
  dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

  VkRenderPassCreateInfo renderPassInfo{};
  renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
  renderPassInfo.attachmentCount = attachments.size();
  renderPassInfo.pAttachments = attachments.data();
  renderPassInfo.subpassCount = 1;
  renderPassInfo.pSubpasses = &subpass;
  renderPassInfo.dependencyCount = dependencies.size();
  renderPassInfo.pDependencies = dependencies.data();

  vkCreateRenderPass(m_rhi->m_device, &renderPassInfo, nullptr, &m_mesh_pass);
}

void MeshPass::CreateDesciptorSetLayout() {
  std::array<VkDescriptorSetLayoutBinding, 2> bindings;
  // perframe data bindings.
  bindings[0].binding = 0;
  bindings[0].descriptorCount = 1;
  bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
  bindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

  // sampled-texture.
  bindings[1].binding = 1;
  bindings[1].descriptorCount = 1;
  bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  bindings[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

  VkDescriptorSetLayoutCreateInfo info{
      VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
  info.bindingCount = bindings.size();
  info.pBindings = bindings.data();

  VK_CHECK(vkCreateDescriptorSetLayout(m_rhi->m_device, &info, nullptr,
                                       &m_mesh_desp_set_layout),
           "Failed to create perframe data desp set.");
}

void MeshPass::CreateVkRenderPipeline() {
  auto device = m_rhi->m_device;
  // set up shader
  VkShaderModule vs = VkUtil::RuntimeCreateShaderModule(
      device, "./engine/shaders/mesh.vert", shaderc_vertex_shader);
  VkShaderModule fs = VkUtil::RuntimeCreateShaderModule(
      device, "./engine/shaders/mesh.frag", shaderc_fragment_shader);

  VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
  vertShaderStageInfo.sType =
      VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
  vertShaderStageInfo.module = vs;
  vertShaderStageInfo.pName = "main";

  VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
  fragShaderStageInfo.sType =
      VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
  fragShaderStageInfo.module = fs;
  fragShaderStageInfo.pName = "main";

  VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo,
                                                    fragShaderStageInfo};

  // setup data
  VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
  pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  pipelineLayoutInfo.pSetLayouts = &m_mesh_desp_set_layout;
  pipelineLayoutInfo.setLayoutCount = 1;
  pipelineLayoutInfo.pushConstantRangeCount = 0;

  VK_CHECK(vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr,
                                  &m_mesh_pipeline_layout),
           "Failed to create pipeline layout.");

  auto input_bindings = RenderMeshVertexBasic::GetInputBindingDescription();
  auto input_attributes = RenderMeshVertexBasic::GetAttributeDescription();
  VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
  vertexInputInfo.sType =
      VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
  vertexInputInfo.pVertexBindingDescriptions = input_bindings.data();
  vertexInputInfo.vertexBindingDescriptionCount = input_bindings.size();
  vertexInputInfo.pVertexAttributeDescriptions = input_attributes.data();
  vertexInputInfo.vertexAttributeDescriptionCount = input_attributes.size();

  // other state config
  VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
  inputAssembly.sType =
      VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
  inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
  inputAssembly.primitiveRestartEnable = VK_FALSE;

  VkViewport viewPort{};
  viewPort.x = 0.f, viewPort.y = 0.f;
  viewPort.width = static_cast<float>(m_rhi->m_swapchain_extent.width);
  viewPort.height = static_cast<float>(m_rhi->m_swapchain_extent.height);
  viewPort.minDepth = 0.f;
  viewPort.maxDepth = 1.f;

  VkPipelineViewportStateCreateInfo viewportState{
      VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO};
  viewportState.viewportCount = 1;
  viewportState.pViewports = &viewPort;
  viewportState.scissorCount = 1;
  viewportState.pScissors = &m_rhi->m_scissor;

  VkPipelineRasterizationStateCreateInfo rasterizer{
      VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO};
  rasterizer.depthClampEnable = VK_FALSE;
  rasterizer.rasterizerDiscardEnable = VK_FALSE;
  rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
  rasterizer.lineWidth = 1.0f;
  rasterizer.cullMode = VK_CULL_MODE_NONE;
  rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
  rasterizer.depthBiasEnable = VK_FALSE;

  VkPipelineDepthStencilStateCreateInfo depth_state{
      VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO};
  depth_state.depthTestEnable = VK_TRUE;
  depth_state.depthWriteEnable = VK_TRUE;
  depth_state.depthCompareOp = VK_COMPARE_OP_LESS;
  depth_state.depthBoundsTestEnable = VK_FALSE;
  depth_state.stencilTestEnable = VK_FALSE;

  VkPipelineMultisampleStateCreateInfo multisampling{};
  multisampling.sType =
      VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
  multisampling.sampleShadingEnable = VK_FALSE;
  multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

  VkPipelineColorBlendAttachmentState colorBlendAttachment{};
  colorBlendAttachment.colorWriteMask =
      VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
      VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
  colorBlendAttachment.blendEnable = VK_FALSE;

  VkPipelineColorBlendStateCreateInfo colorBlending{};
  colorBlending.sType =
      VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
  colorBlending.logicOpEnable = VK_FALSE;
  colorBlending.attachmentCount = 1;
  colorBlending.pAttachments = &colorBlendAttachment;

  VkGraphicsPipelineCreateInfo pipelineInfo{};
  pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
  pipelineInfo.stageCount = 2;
  pipelineInfo.pStages = shaderStages;
  pipelineInfo.pVertexInputState = &vertexInputInfo;
  pipelineInfo.pInputAssemblyState = &inputAssembly;
  pipelineInfo.pViewportState = &viewportState;
  pipelineInfo.pRasterizationState = &rasterizer;
  pipelineInfo.pDepthStencilState = &depth_state;
  pipelineInfo.pMultisampleState = &multisampling;
  pipelineInfo.pColorBlendState = &colorBlending;
  pipelineInfo.layout = m_mesh_pipeline_layout;
  pipelineInfo.renderPass = m_mesh_pass;
  pipelineInfo.subpass = 0;
  pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

  VK_CHECK(vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo,
                                     nullptr, &m_mesh_pipeline),
           "Failed to create mesh pipeline!");

  vkDestroyShaderModule(device, vs, nullptr);
  vkDestroyShaderModule(device, fs, nullptr);
}

void MeshPass::CreateDesciptorSet() {
  // allocate set.
  VkDescriptorSetAllocateInfo info{
      VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
  info.descriptorPool = m_rhi->m_descriptor_pool;
  info.descriptorSetCount = 1;
  info.pSetLayouts = &m_mesh_desp_set_layout;

  VK_CHECK(vkAllocateDescriptorSets(m_rhi->m_device, &info, &m_mesh_desp_set),
           "Failed to create desp set.");

  // perframe buffer set.
  VkDescriptorBufferInfo perframe_data_buf_info{};
  perframe_data_buf_info.offset = 0;
  perframe_data_buf_info.range = sizeof(PerframeData);
  perframe_data_buf_info.buffer =
      m_resources->GetPerframeDataObject().perframe_data_buffer;

  // mesh texture set
  VkDescriptorImageInfo mesh_texture_info{};
  mesh_texture_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
  mesh_texture_info.imageView =
      m_resources->GetTerrainTextureObject().texture_image_view;
  mesh_texture_info.sampler = m_resources->GetTerrainSampler();

  std::array<VkWriteDescriptorSet, 2> writers;
  // perframe data writer
  writers[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  writers[0].pNext = nullptr;
  writers[0].dstSet = m_mesh_desp_set;
  writers[0].dstBinding = 0;
  writers[0].dstArrayElement = 0;
  writers[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
  writers[0].descriptorCount = 1;
  writers[0].pBufferInfo = &perframe_data_buf_info;

  writers[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  writers[1].pNext = nullptr;
  writers[1].dstSet = m_mesh_desp_set;
  writers[1].dstBinding = 1;
  writers[1].dstArrayElement = 0;
  writers[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  writers[1].descriptorCount = 1;
  writers[1].pImageInfo = &mesh_texture_info;

  vkUpdateDescriptorSets(m_rhi->m_device, writers.size(), writers.data(), 0,
                         nullptr);
}

void MeshPass::CreateFrameBuffers() {
  m_framebuffers.resize(m_rhi->m_swapchain_images.size());
  uint32_t width = m_rhi->m_swapchain_extent.width;
  uint32_t height = m_rhi->m_swapchain_extent.height;
  for (size_t i = 0; i < m_framebuffers.size(); ++i) {
    VkImageView attachments[] = {
        m_rhi->m_swapchain_imageviews[i],
        m_depth_objects[i].depth_image_view,
    };

    VkFramebufferCreateInfo framebufferInfo{
        .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
        .renderPass = m_mesh_pass,
        .attachmentCount = 2,
        .pAttachments = attachments,
        .width = width,
        .height = height,
        .layers = 1,
    };

    VK_CHECK(vkCreateFramebuffer(m_rhi->m_device, &framebufferInfo, nullptr,
                                 &m_framebuffers[i]),
             "Failed to create framebuffer!");
  }
}

}  // namespace ShaderStory