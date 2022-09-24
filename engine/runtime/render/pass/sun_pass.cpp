#include "engine/runtime/render/pass/sun_pass.hpp"

#include <iostream>

#include "engine/runtime/render/rhi/vulkan/vk_utils.hpp"
namespace ShaderStory {

SunPass::SunPass() { std::cout << "create sunpass.\n"; }

SunPass::~SunPass() { std::cout << "destory sunpass.\n"; }

void SunPass::Initialze() {
  CreateVkRenderPass();
  CreateDesciptorSetLayout();
  CreateVkRenderPipeline();
  CreateDesciptorSet();
  CreateFrameBuffers();
}

void SunPass::Dispose() {}

void SunPass::RunPass() {
  VkCommandBuffer command_buffer = m_rhi->GetCurrentCommandBuffer();

  VkRenderPassBeginInfo renderPassInfo{};
  renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  renderPassInfo.renderPass = m_sun_shadowmap_pass;
  renderPassInfo.renderArea.offset = {0, 0};
  renderPassInfo.renderArea.extent.width =
      m_resources->GetSunResourceObject().shadowmap_width;
  renderPassInfo.renderArea.extent.height =
      m_resources->GetSunResourceObject().shadowmap_height;

  std::array<VkClearValue, 1> clear_vals;
  clear_vals[0].depthStencil = {1.f, 0};
  renderPassInfo.clearValueCount = clear_vals.size();
  renderPassInfo.pClearValues = clear_vals.data();

  for (int i = 0; i < SunResourceObject::SHADOWMAP_CNT; ++i) {
    renderPassInfo.framebuffer =
        m_cascades[m_rhi->m_current_swapchain_image_index].cascade_fbs[i];
    vkCmdBeginRenderPass(command_buffer, &renderPassInfo,
                         VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                      m_sun_shadowmap_pipeline);

    u_int32_t cur_frame_idx = m_rhi->GetCurrentFrameIndex();
    size_t offset = m_resources->GetPerframeDataObject().GetOffset();
    u_int32_t dy_offsets = cur_frame_idx * offset;

    PushConstantBlock constant_block{};
    constant_block.cascade_index = i;
    assert(constant_block.cascade_index >= 0 &&
           constant_block.cascade_index < 3);

    vkCmdPushConstants(command_buffer, m_sun_shadowmap_pipeline_layout,
                       VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(constant_block),
                       &constant_block);

    vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                            m_sun_shadowmap_pipeline_layout, 0, 1,
                            &m_shadowmap_desp_set, 1, &dy_offsets);

    VkDeviceSize offsets[] = {0};
    for (const auto& [k, v] : m_resources->GetMeshesObject()) {
      vkCmdBindVertexBuffers(command_buffer, 0, 1, &v.mesh_vert_buf, offsets);
      vkCmdBindIndexBuffer(command_buffer, v.mesh_indices_buf, 0,
                           VK_INDEX_TYPE_UINT32);
      vkCmdDrawIndexed(command_buffer, v.index_count, 1, 0, 0, 0);
    }

    vkCmdEndRenderPass(m_rhi->GetCurrentCommandBuffer());
  }
}

void SunPass::CreateVkRenderPass() {
  std::array<VkAttachmentDescription, 1> attachments;

  // depth attach
  attachments[0].format = m_resources->GetSunResourceObject().shadow_map_format;
  attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
  attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  attachments[0].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;

  std::array<VkAttachmentReference, 1> attachment_refs;

  // depth attachment ref
  attachment_refs[0].attachment = 0;
  attachment_refs[0].layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

  VkSubpassDescription subpass{};
  subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
  subpass.colorAttachmentCount = 0;
  subpass.inputAttachmentCount = 0;
  subpass.pDepthStencilAttachment = &attachment_refs[0];

  std::array<VkSubpassDependency, 2> dependencies;
  // for wrtie
  dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
  dependencies[0].dstSubpass = 0;
  dependencies[0].srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
  dependencies[0].dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
  dependencies[0].srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
  dependencies[0].dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
  dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

  // for read
  dependencies[1].srcSubpass = 0;
  dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
  dependencies[1].srcStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
  dependencies[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
  dependencies[1].srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
  dependencies[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
  dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

  VkRenderPassCreateInfo renderPassInfo{};
  renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
  renderPassInfo.attachmentCount = attachments.size();
  renderPassInfo.pAttachments = attachments.data();
  renderPassInfo.subpassCount = 1;
  renderPassInfo.pSubpasses = &subpass;
  renderPassInfo.dependencyCount = dependencies.size();
  renderPassInfo.pDependencies = dependencies.data();

  vkCreateRenderPass(m_rhi->m_device, &renderPassInfo, nullptr,
                     &m_sun_shadowmap_pass);
}

void SunPass::CreateDesciptorSetLayout() {
  // only need light projview matrix.
  std::array<VkDescriptorSetLayoutBinding, 1> bindings;
  // perframe data bindings.
  bindings[0].binding = 0;
  bindings[0].descriptorCount = 1;
  bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
  bindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

  VkDescriptorSetLayoutCreateInfo info{
      VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
  info.bindingCount = bindings.size();
  info.pBindings = bindings.data();

  VK_CHECK(vkCreateDescriptorSetLayout(m_rhi->m_device, &info, nullptr,
                                       &m_sun_shadowmap_desp_set_layout),
           "Failed to create perframe data desp set.");
}

void SunPass::CreateVkRenderPipeline() {
  auto device = m_rhi->m_device;
  // set up shader
  VkShaderModule vs = VkUtil::RuntimeCreateShaderModule(
      device, "./shaders/sun.vert", shaderc_vertex_shader);
  VkShaderModule fs = VkUtil::RuntimeCreateShaderModule(
      device, "./shaders/sun.frag", shaderc_fragment_shader);

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

  VkPushConstantRange constant_range;
  constant_range.offset = 0;
  constant_range.size = sizeof(PushConstantBlock);
  constant_range.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

  // setup data
  VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
  pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  pipelineLayoutInfo.pSetLayouts = &m_sun_shadowmap_desp_set_layout;
  pipelineLayoutInfo.setLayoutCount = 1;
  pipelineLayoutInfo.pushConstantRangeCount = 1;
  pipelineLayoutInfo.pPushConstantRanges = &constant_range;

  VK_CHECK(vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr,
                                  &m_sun_shadowmap_pipeline_layout),
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
  viewPort.x = 0.f;
  viewPort.y = 0.f;
  viewPort.width =
      static_cast<float>(m_resources->GetSunResourceObject().shadowmap_width);
  viewPort.height =
      static_cast<float>(m_resources->GetSunResourceObject().shadowmap_width);
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
  pipelineInfo.layout = m_sun_shadowmap_pipeline_layout;
  pipelineInfo.renderPass = m_sun_shadowmap_pass;
  pipelineInfo.subpass = 0;
  pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

  VK_CHECK(vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo,
                                     nullptr, &m_sun_shadowmap_pipeline),
           "Failed to create mesh pipeline!");

  vkDestroyShaderModule(device, vs, nullptr);
  vkDestroyShaderModule(device, fs, nullptr);
}

void SunPass::CreateDesciptorSet() {
  // allocate set.
  VkDescriptorSetAllocateInfo info{
      VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
  info.descriptorPool = m_rhi->m_descriptor_pool;
  info.descriptorSetCount = 1;
  info.pSetLayouts = &m_sun_shadowmap_desp_set_layout;

  VK_CHECK(
      vkAllocateDescriptorSets(m_rhi->m_device, &info, &m_shadowmap_desp_set),
      "Failed to create desp set.");

  // perframe buffer set.
  VkDescriptorBufferInfo perframe_data_buf_info{};
  perframe_data_buf_info.offset = 0;
  perframe_data_buf_info.range = sizeof(PerframeData);
  perframe_data_buf_info.buffer =
      m_resources->GetPerframeDataObject().perframe_data_buffer;

  std::array<VkWriteDescriptorSet, 1> writers;
  // perframe data writer
  writers[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  writers[0].pNext = nullptr;
  writers[0].dstSet = m_shadowmap_desp_set;
  writers[0].dstBinding = 0;
  writers[0].dstArrayElement = 0;
  writers[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
  writers[0].descriptorCount = 1;
  writers[0].pBufferInfo = &perframe_data_buf_info;

  vkUpdateDescriptorSets(m_rhi->m_device, writers.size(), writers.data(), 0,
                         nullptr);
}

void SunPass::CreateFrameBuffers() {
  // i frame, j cascasde
  for (size_t i = 0; i < m_cascades.size(); ++i) {
    for (size_t j = 0; j < m_cascades[i].cascade_fbs.size(); ++j) {
      VkImageView attachments[] = {
          m_resources->GetSunResourceObject()
              .sun_depth[i]
              .cascade_shadowmap_views[j],
      };

      VkFramebufferCreateInfo framebufferInfo{
          .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
          .renderPass = m_sun_shadowmap_pass,
          .attachmentCount = 1,
          .pAttachments = attachments,
          .width = m_resources->GetSunResourceObject().shadowmap_width,
          .height = m_resources->GetSunResourceObject().shadowmap_height,
          .layers = 1,
      };

      VK_CHECK(vkCreateFramebuffer(m_rhi->m_device, &framebufferInfo, nullptr,
                                   &m_cascades[i].cascade_fbs[j]),
               "Failed to create framebuffer!");
    }
  }
}

}  // namespace ShaderStory