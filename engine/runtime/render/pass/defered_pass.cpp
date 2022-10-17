#include "engine/runtime/render/pass/defered_pass.hpp"

#include "engine/runtime/render/render_resource.hpp"
#include "engine/runtime/render/rhi/vulkan/vk_utils.hpp"

namespace ShaderStory {

DeferedPass::DeferedPass() {}

DeferedPass::~DeferedPass() {}

void DeferedPass::RunPass() {
  VkCommandBuffer command_buffer = m_rhi->GetCurrentCommandBuffer();

  VkRenderPassBeginInfo renderPassInfo{};
  renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  renderPassInfo.renderPass = m_offscreen_pass;
  renderPassInfo.framebuffer =
      m_offscreen_framebuffers[m_rhi->m_current_swapchain_image_index];
  renderPassInfo.renderArea.offset = {0, 0};
  renderPassInfo.renderArea.extent = m_rhi->m_swapchain_extent;

  std::array<VkClearValue, 5> clear_vals;
  clear_vals[0].color = {{1.0f, 0.0f, 1.0f, 1.0f}};  // pos
  clear_vals[1].color = {{0.0f, 0.0f, 0.0f, 1.0f}};  // normal
  clear_vals[2].color = {{1.0f, 0.0f, 1.0f, 1.0f}};  // albedo
  clear_vals[3].color = {{1.0f, 0.0f, 1.0f, 1.0f}};  // materia;
  clear_vals[4].depthStencil = {1.f, 0};
  renderPassInfo.clearValueCount = clear_vals.size();
  renderPassInfo.pClearValues = clear_vals.data();

  vkCmdBeginRenderPass(command_buffer, &renderPassInfo,
                       VK_SUBPASS_CONTENTS_INLINE);

  vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                    m_offscreen_pipeline);

  u_int32_t cur_frame_idx = m_rhi->GetCurrentFrameIndex();
  size_t offset =
      m_resources->GetPerframeUBOManager().GetPerframeUBODynamicOffset();
  u_int32_t dy_offsets = cur_frame_idx * offset;

  vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                          m_offscreen_pipeline_layout, 0, 1, &m_offscreen_set,
                          1, &dy_offsets);

  VkDeviceSize offsets[] = {0};

  for (const auto& [k, v] : m_resources->GetMeshesObject()) {
    vkCmdBindVertexBuffers(command_buffer, 0, 1, &v.mesh_vert_buf, offsets);
    vkCmdBindIndexBuffer(command_buffer, v.mesh_indices_buf, 0,
                         VK_INDEX_TYPE_UINT32);
    vkCmdDrawIndexed(command_buffer, v.index_count, 1, 0, 0, 0);
  }

  vkCmdEndRenderPass(command_buffer);
}

void DeferedPass::Initialize() {
  CreateVkRenderPass();
  CreateDesciptorSetLayout();
  CreateVkRenderPipeline();
  CreateDesciptorSet();
  CreateFrameBuffers();
}

void DeferedPass::Dispose() {}

void DeferedPass::CreateVkRenderPass() {
  std::array<VkAttachmentDescription, 5> attachments;
  // format is setup in image creation.
  // position
  attachments[0].format = VK_FORMAT_R16G16B16A16_SFLOAT;
  attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
  attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  attachments[0].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
  attachments[0].flags = 0;

  // normal
  attachments[1].format = VK_FORMAT_R16G16B16A16_SFLOAT;
  attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
  attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  attachments[1].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
  attachments[1].flags = 0;

  // albedo
  attachments[2].format = VK_FORMAT_R8G8B8A8_UNORM;
  attachments[2].samples = VK_SAMPLE_COUNT_1_BIT;
  attachments[2].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  attachments[2].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  attachments[2].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  attachments[2].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  attachments[2].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  attachments[2].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
  attachments[2].flags = 0;

  // pbr material
  attachments[3].format = VK_FORMAT_R32G32B32A32_SFLOAT;
  attachments[3].samples = VK_SAMPLE_COUNT_1_BIT;
  attachments[3].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  attachments[3].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  attachments[3].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  attachments[3].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  attachments[3].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  attachments[3].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
  attachments[3].flags = 0;

  // depth
  attachments[4].format =
      m_resources->GetDeferedResourceManager().GetDepthFormat();
  attachments[4].samples = VK_SAMPLE_COUNT_1_BIT;
  attachments[4].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  attachments[4].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  attachments[4].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  attachments[4].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  attachments[4].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  attachments[4].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
  attachments[4].flags = 0;

  std::array<VkAttachmentReference, 4> attach_refs;
  attach_refs[0].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
  attach_refs[0].attachment = 0;
  attach_refs[1].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
  attach_refs[1].attachment = 1;
  attach_refs[2].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
  attach_refs[2].attachment = 2;
  // pbr
  attach_refs[3].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
  attach_refs[3].attachment = 3;

  VkAttachmentReference depth_ref;
  depth_ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
  depth_ref.attachment = 4;

  VkSubpassDescription subpass{};
  subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
  subpass.colorAttachmentCount = attach_refs.size();
  subpass.pColorAttachments = attach_refs.data();
  subpass.inputAttachmentCount = 0;
  subpass.pDepthStencilAttachment = &depth_ref;

  std::array<VkSubpassDependency, 2> dependencies;
  dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
  dependencies[0].dstSubpass = 0;
  dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
  dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
  dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
                                  VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
  dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

  dependencies[1].srcSubpass = VK_SUBPASS_EXTERNAL;
  dependencies[1].dstSubpass = 0;
  dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
  dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
                                  VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

  dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
  dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

  // std::array<VkSubpassDependency, 2> dependencies;
  // // for wrtie
  // dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
  // dependencies[0].dstSubpass = 0;
  // dependencies[0].srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
  // dependencies[0].dstStageMask =
  // VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT; dependencies[0].srcAccessMask
  // = VK_ACCESS_SHADER_READ_BIT; dependencies[0].dstAccessMask =
  //     VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
  // dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

  // // for read
  // dependencies[1].srcSubpass = 0;
  // dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
  // dependencies[1].srcStageMask =
  // VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT; dependencies[1].dstStageMask
  // = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT; dependencies[1].srcAccessMask =
  //     VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
  // dependencies[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
  // dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

  VkRenderPassCreateInfo renderPassInfo{};
  renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
  renderPassInfo.attachmentCount = attachments.size();
  renderPassInfo.pAttachments = attachments.data();
  renderPassInfo.subpassCount = 1;
  renderPassInfo.pSubpasses = &subpass;
  renderPassInfo.dependencyCount = dependencies.size();
  renderPassInfo.pDependencies = dependencies.data();

  VK_CHECK(vkCreateRenderPass(m_rhi->m_device, &renderPassInfo, nullptr,
                              &m_offscreen_pass),
           "Create Offscreen renderpass failed.");
}

void DeferedPass::CreateDesciptorSetLayout() {
  std::array<VkDescriptorSetLayoutBinding, 4> bindings;
  // perframe data bindings.
  bindings[0].binding = 0;
  bindings[0].descriptorCount = 1;
  bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
  bindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

  // terrain-texture.
  bindings[1].binding = 1;
  bindings[1].descriptorCount = 1;
  bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  bindings[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
  bindings[1].pImmutableSamplers = nullptr;

  // terrain-normal_map
  bindings[2].binding = 2;
  bindings[2].descriptorCount = 1;
  bindings[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  bindings[2].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
  bindings[2].pImmutableSamplers = nullptr;

  // terrain-pbr-materials
  bindings[3].binding = 3;
  bindings[3].descriptorCount = 1;
  bindings[3].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  bindings[3].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
  bindings[3].pImmutableSamplers = nullptr;

  VkDescriptorSetLayoutCreateInfo info{
      VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
  info.bindingCount = bindings.size();
  info.pBindings = bindings.data();

  VK_CHECK(vkCreateDescriptorSetLayout(m_rhi->m_device, &info, nullptr,
                                       &m_offscreen_set_layout),
           "Failed to create offscreen desp set.");
}

void DeferedPass::CreateVkRenderPipeline() {
  VkDevice device = m_rhi->m_device;

  // set up shader
  VkShaderModule vs = VkUtil::RuntimeCreateShaderModule(
      device, "./shaders/defered.vert", shaderc_vertex_shader);
  VkShaderModule fs = VkUtil::RuntimeCreateShaderModule(
      device, "./shaders/defered.frag", shaderc_fragment_shader);

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
  pipelineLayoutInfo.pSetLayouts = &m_offscreen_set_layout;
  pipelineLayoutInfo.setLayoutCount = 1;
  pipelineLayoutInfo.pushConstantRangeCount = 0;

  VK_CHECK(vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr,
                                  &m_offscreen_pipeline_layout),
           "Failed to create offscreen pipeline layout.");

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
  viewPort.x = 0.f;
  viewPort.y = 0.f;
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

  // we have 4 color attachments
  std::array<VkPipelineColorBlendAttachmentState, 4> blend_attachment_states;

  VkPipelineColorBlendAttachmentState colorBlendAttachment{};
  colorBlendAttachment.colorWriteMask =
      VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
      VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
  colorBlendAttachment.blendEnable = VK_FALSE;

  for (auto& state : blend_attachment_states) {
    state = colorBlendAttachment;
  }

  VkPipelineColorBlendStateCreateInfo colorBlending{};
  colorBlending.sType =
      VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
  colorBlending.logicOpEnable = VK_FALSE;
  colorBlending.attachmentCount = blend_attachment_states.size();
  colorBlending.pAttachments = blend_attachment_states.data();

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
  pipelineInfo.layout = m_offscreen_pipeline_layout;
  pipelineInfo.renderPass = m_offscreen_pass;
  pipelineInfo.subpass = 0;
  pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

  VK_CHECK(vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo,
                                     nullptr, &m_offscreen_pipeline),
           "Failed to create offscreen pipeline!");

  vkDestroyShaderModule(device, vs, nullptr);
  vkDestroyShaderModule(device, fs, nullptr);
}

void DeferedPass::CreateDesciptorSet() {
  // allocate set.
  VkDescriptorSetAllocateInfo info{
      VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
  info.descriptorPool = m_rhi->m_descriptor_pool;
  info.descriptorSetCount = 1;
  info.pSetLayouts = &m_offscreen_set_layout;

  VK_CHECK(vkAllocateDescriptorSets(m_rhi->m_device, &info, &m_offscreen_set),
           "Failed to create desp set.");

  // perframe buffer set. (0)
  VkDescriptorBufferInfo perframe_data_buf_info =
      m_resources->GetPerframeUBOManager().GetDespBufInfo();

  // mesh texture set (1)
  VkDescriptorImageInfo terrain_texture_info =
      m_resources->GetTerrainMaterialManager().GetAlbedoDespImageInfo();
  terrain_texture_info.sampler = m_resources->GetTerrainSampler();

  // terrain normalmap (2)
  VkDescriptorImageInfo terrain_normalmap_info =
      m_resources->GetTerrainMaterialManager().GetNormalMapDespImageInfo();
  terrain_normalmap_info.sampler = m_resources->GetDefaultNearestSampler();

  // terrain pbr (3)
  VkDescriptorImageInfo pbr_material_info =
      m_resources->GetTerrainMaterialManager().GetMaterialDespImageInfo();
  pbr_material_info.sampler = m_resources->GetDefaultNearestSampler();

  std::array<VkWriteDescriptorSet, 4> writers;
  // perframe data writer
  writers[0] = m_resources->GetPerframeUBOManager().GetDespWrite();
  writers[0].dstSet = m_offscreen_set;
  writers[0].dstBinding = 0;
  writers[0].pBufferInfo = &perframe_data_buf_info;

  // terrain writer
  writers[1] = m_resources->GetTerrainMaterialManager().GetMaterialDespWrite();
  writers[1].dstSet = m_offscreen_set;
  writers[1].dstBinding = 1;
  writers[1].pImageInfo = &terrain_texture_info;

  // terrain normalmap
  writers[2] =
      m_resources->GetTerrainMaterialManager().GetNormalMapImageWrite();
  writers[2].dstSet = m_offscreen_set;
  writers[2].dstBinding = 2;
  writers[2].pImageInfo = &terrain_normalmap_info;

  // terrain pbr material
  writers[3] = m_resources->GetTerrainMaterialManager().GetMaterialDespWrite();
  writers[3].dstSet = m_offscreen_set;
  writers[3].dstBinding = 3;
  writers[3].pImageInfo = &pbr_material_info;

  vkUpdateDescriptorSets(m_rhi->m_device, writers.size(), writers.data(), 0,
                         nullptr);
}

void DeferedPass::CreateFrameBuffers() {
  VkFramebufferCreateInfo fbf_info = {};
  fbf_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
  fbf_info.pNext = nullptr;
  fbf_info.renderPass = m_offscreen_pass;
  fbf_info.width = m_rhi->m_swapchain_extent.width;
  fbf_info.height = m_rhi->m_swapchain_extent.height;
  fbf_info.layers = 1;

  const auto& gbuffer = m_resources->GetDeferedResourceManager();
  for (int i = 0; i < m_offscreen_framebuffers.size(); ++i) {
    std::array<VkImageView, 5> attachments;
    attachments[0] = gbuffer.GetPositionImageView(i);
    attachments[1] = gbuffer.GetNormalImageView(i);
    attachments[2] = gbuffer.GetAlbedoImageView(i);
    attachments[3] = gbuffer.GetPBRMaterialView(i);
    attachments[4] = gbuffer.GetDepthImageView(i);

    fbf_info.attachmentCount = attachments.size();
    fbf_info.pAttachments = attachments.data();

    VK_CHECK(vkCreateFramebuffer(m_rhi->m_device, &fbf_info, nullptr,
                                 &m_offscreen_framebuffers[i]),
             "Failed to create framebuffer!");
  }
}

void DeferedPass::CreateLightEntityPipeline() {}
void DeferedPass::CreateLightEntitySetLayout() {}
void DeferedPass::CreateLightDescriptorSet() {}

}  // namespace ShaderStory