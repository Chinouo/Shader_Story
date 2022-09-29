#include "engine/runtime/render/pass/ssao_pass.hpp"

#include <array>

#include "engine/runtime/render/render_resource.hpp"
#include "engine/runtime/render/rhi/vulkan/vk_utils.hpp"

namespace ShaderStory {

SSAOPass::SSAOPass() {}

SSAOPass::~SSAOPass() {}

void SSAOPass::RunPass() {
  VkCommandBuffer command_buffer = m_rhi->GetCurrentCommandBuffer();

  VkRenderPassBeginInfo renderPassInfo{};
  renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  renderPassInfo.renderPass = m_ssao_pass;
  renderPassInfo.framebuffer =
      m_ssao_fbs[m_rhi->m_current_swapchain_image_index];
  renderPassInfo.renderArea.offset = {0, 0};
  renderPassInfo.renderArea.extent = m_rhi->m_swapchain_extent;

  std::array<VkClearValue, 1> clear_vals;
  clear_vals[0].color = {{0.0f, 0.0f, 1.0f, 1.0f}};

  renderPassInfo.clearValueCount = clear_vals.size();
  renderPassInfo.pClearValues = clear_vals.data();

  vkCmdBeginRenderPass(command_buffer, &renderPassInfo,
                       VK_SUBPASS_CONTENTS_INLINE);

  vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                    m_ssao_pip);

  u_int32_t cur_frame_idx = m_rhi->GetCurrentFrameIndex();
  size_t offset =
      m_resources->GetPerframeUBOManager().GetPerframeUBODynamicOffset();
  u_int32_t dy_offsets = cur_frame_idx * offset;

  vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                          m_ssao_pip_layout, 0, 1, &m_ssao_sets[cur_frame_idx],
                          1, &dy_offsets);

  vkCmdDraw(command_buffer, 6, 1, 0, 1);
  vkCmdEndRenderPass(command_buffer);
}

void SSAOPass::Initialize() {
  CreateVkRenderPass();
  CreateDesciptorSetLayout();
  CreateVkRenderPipeline();
  CreateDesciptorSet();
  CreateFrameBuffers();
}

void SSAOPass::Dispose() {}

void SSAOPass::CreateVkRenderPass() {
  // ssao attachement
  std::array<VkAttachmentDescription, 1> attachments;

  attachments[0].format = VK_FORMAT_R8_UNORM;
  attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
  attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  attachments[0].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
  attachments[0].flags = 0;

  std::array<VkAttachmentReference, 1> attachment_refs;
  attachment_refs[0].attachment = 0;
  attachment_refs[0].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

  VkSubpassDescription subpass{};
  subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
  subpass.colorAttachmentCount = 1;
  subpass.inputAttachmentCount = 0;
  subpass.pColorAttachments = attachment_refs.data();
  subpass.pDepthStencilAttachment = nullptr;

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

  vkCreateRenderPass(m_rhi->m_device, &renderPassInfo, nullptr, &m_ssao_pass);
}

void SSAOPass::CreateDesciptorSetLayout() {
  std::array<VkDescriptorSetLayoutBinding, 5> bindings;
  // perframe buffer(UBO)
  bindings[0].binding = 0;
  bindings[0].descriptorCount = 1;
  bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
  bindings[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
  bindings[0].pImmutableSamplers = nullptr;

  // g-position vs
  bindings[1].binding = 1;
  bindings[1].descriptorCount = 1;
  bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  bindings[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
  bindings[1].pImmutableSamplers = nullptr;

  // g-normal
  bindings[2].binding = 2;
  bindings[2].descriptorCount = 1;
  bindings[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  bindings[2].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
  bindings[2].pImmutableSamplers = nullptr;

  // ssao kernal
  bindings[3].binding = 3;
  bindings[3].descriptorCount = 1;
  bindings[3].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
  bindings[3].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
  bindings[3].pImmutableSamplers = nullptr;

  // ssao noise
  bindings[4].binding = 4;
  bindings[4].descriptorCount = 1;
  bindings[4].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  bindings[4].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
  bindings[4].pImmutableSamplers = nullptr;

  VkDescriptorSetLayoutCreateInfo info{};
  info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  info.bindingCount = bindings.size();
  info.pBindings = bindings.data();

  VK_CHECK(vkCreateDescriptorSetLayout(m_rhi->m_device, &info, nullptr,
                                       &m_ssao_set_layout),
           "Failed to create offscreen desp set.");
}

void SSAOPass::CreateVkRenderPipeline() {
  VkDevice device = m_rhi->m_device;

  shaderc_include_result res;

  // set up shader
  VkShaderModule vs = VkUtil::RuntimeCreateShaderModule(
      device, "./shaders/ssao.vert", shaderc_vertex_shader);
  VkShaderModule fs = VkUtil::RuntimeCreateShaderModule(
      device, "./shaders/ssao.frag", shaderc_fragment_shader);

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
  pipelineLayoutInfo.pSetLayouts = &m_ssao_set_layout;
  pipelineLayoutInfo.setLayoutCount = 1;
  pipelineLayoutInfo.pushConstantRangeCount = 0;

  VK_CHECK(vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr,
                                  &m_ssao_pip_layout),
           "Failed to create ssao pipeline layout.");

  VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
  vertexInputInfo.sType =
      VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
  vertexInputInfo.pVertexBindingDescriptions = nullptr;
  vertexInputInfo.vertexBindingDescriptionCount = 0;
  vertexInputInfo.pVertexAttributeDescriptions = nullptr;
  vertexInputInfo.vertexAttributeDescriptionCount = 0;

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
  pipelineInfo.layout = m_ssao_pip_layout;
  pipelineInfo.renderPass = m_ssao_pass;
  pipelineInfo.subpass = 0;
  pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

  VK_CHECK(vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo,
                                     nullptr, &m_ssao_pip),
           "Failed to create ssao pipeline!");

  vkDestroyShaderModule(device, vs, nullptr);
  vkDestroyShaderModule(device, fs, nullptr);
}

void SSAOPass::CreateDesciptorSet() {
  std::vector<VkDescriptorSetLayout> set_layouts(MAX_FRAMES_IN_FLIGHT,
                                                 m_ssao_set_layout);

  VkDescriptorSetAllocateInfo info{};
  info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  info.descriptorPool = m_rhi->m_descriptor_pool;
  info.descriptorSetCount = m_ssao_sets.size();
  info.pSetLayouts = set_layouts.data();

  VK_CHECK(vkAllocateDescriptorSets(m_rhi->m_device, &info, m_ssao_sets.data()),
           "Failed to create desp set.");

  // perframe ubo
  VkDescriptorBufferInfo perframe_ubo_info =
      m_resources->GetPerframeUBOManager().GetDespBufInfo();

  std::array<VkWriteDescriptorSet, 5> writers;
  writers[0] = m_resources->GetPerframeUBOManager().GetDespWrite();
  writers[0].dstBinding = 0;
  writers[0].pBufferInfo = &perframe_ubo_info;

  const auto& gbuffer = m_resources->GetDeferedResourceManager();
  writers[1] = gbuffer.GetPositionDespWrite();
  writers[1].dstBinding = 1;

  writers[2] = gbuffer.GetNormalDespWrite();
  writers[2].dstBinding = 2;

  VkDescriptorBufferInfo ssao_kernal_info =
      m_resources->GetSSAOResourceManager().GetSSAOKernalBufInfo();
  writers[3] = m_resources->GetSSAOResourceManager().GetSSAOKernalWriteInfo();
  writers[3].dstBinding = 3;
  writers[3].pBufferInfo = &ssao_kernal_info;

  VkDescriptorImageInfo ssao_noise_info =
      m_resources->GetSSAOResourceManager().GetSSAONoiseImageInfo();
  writers[4] = m_resources->GetSSAOResourceManager().GetSSAONoiseWriteInfo();
  writers[4].dstBinding = 4;
  writers[4].pImageInfo = &ssao_noise_info;

  for (size_t i = 0; i < m_ssao_sets.size(); ++i) {
    VkDescriptorImageInfo g_position_info = gbuffer.GetPositionDespImageInfo(i);
    VkDescriptorImageInfo g_normal_info = gbuffer.GetNormalDespImageInfo(i);

    writers[0].dstSet = m_ssao_sets[i];

    writers[1].dstSet = m_ssao_sets[i];
    writers[1].pImageInfo = &g_position_info;

    writers[2].dstSet = m_ssao_sets[i];
    writers[2].pImageInfo = &g_normal_info;

    writers[3].dstSet = m_ssao_sets[i];

    writers[4].dstSet = m_ssao_sets[i];

    vkUpdateDescriptorSets(m_rhi->m_device, writers.size(), writers.data(), 0,
                           nullptr);
  }
}

void SSAOPass::CreateFrameBuffers() {
  VkFramebufferCreateInfo fbf_info = {};
  fbf_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
  fbf_info.pNext = nullptr;
  fbf_info.renderPass = m_ssao_pass;
  fbf_info.width = m_rhi->m_swapchain_extent.width;
  fbf_info.height = m_rhi->m_swapchain_extent.height;
  fbf_info.layers = 1;

  for (int i = 0; i < m_ssao_fbs.size(); ++i) {
    std::array<VkImageView, 1> attachments;
    attachments[0] = m_resources->GetSSAOResourceManager()
                         .GetSSAOResources(i)
                         .ssao_image_view;

    fbf_info.attachmentCount = attachments.size();
    fbf_info.pAttachments = attachments.data();

    VK_CHECK(vkCreateFramebuffer(m_rhi->m_device, &fbf_info, nullptr,
                                 &m_ssao_fbs[i]),
             "Failed to create framebuffer!");
  }
}

}  // namespace ShaderStory