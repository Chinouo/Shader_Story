
#include "engine/runtime/render/pass/test_pass.hpp"

#include "engine/common/macros.h"
#include "engine/runtime/render/rhi/vulkan/vk_rhi.hpp"
#include "engine/runtime/render/rhi/vulkan/vk_utils.hpp"

namespace ShaderStory {

void TestPass::Dispose() {
  VkDevice device = m_rhi->m_device;
  for (size_t i = 0; i < m_framebuffers.size(); ++i) {
    if (m_framebuffers[i])
      vkDestroyFramebuffer(device, m_framebuffers[i], nullptr);
  }
  if (m_test_pass) vkDestroyRenderPass(device, m_test_pass, nullptr);
  if (m_test_pipeline_layout)
    vkDestroyPipelineLayout(device, m_test_pipeline_layout, nullptr);
  if (m_test_pipeline) vkDestroyPipeline(device, m_test_pipeline, nullptr);
}

void TestPass::Initialize() {
  CreateVkRenderPass();
  CreateVkRenderPipeline();
  CreateFrameBuffers();
}

void TestPass::RunPass() {
  VkCommandBuffer command_buffer = m_rhi->GetCurrentCommandBuffer();

  VkRenderPassBeginInfo renderPassInfo{};
  renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  renderPassInfo.renderPass = m_test_pass;
  renderPassInfo.framebuffer =
      m_framebuffers[m_rhi->m_current_swapchain_image_index];
  renderPassInfo.renderArea.offset = {0, 0};
  renderPassInfo.renderArea.extent = m_rhi->m_swapchain_extent;

  VkClearValue clearColor = {{{0.3f, 0.4f, 0.5f, 1.0f}}};
  renderPassInfo.clearValueCount = 1;
  renderPassInfo.pClearValues = &clearColor;

  vkCmdBeginRenderPass(command_buffer, &renderPassInfo,
                       VK_SUBPASS_CONTENTS_INLINE);

  vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                    m_test_pipeline);

  vkCmdDraw(command_buffer, 3, 1, 0, 0);

  // NOTE: GIVEN TO UI PASS TO END.
  // vkCmdEndRenderPass(command_buffer);
}

void TestPass::CreateVkRenderPass() {
  VkAttachmentDescription colorAttachment{};
  colorAttachment.format = m_rhi->m_swapchain_format;
  colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
  colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

  VkAttachmentReference colorAttachmentRef{};
  colorAttachmentRef.attachment = 0;
  colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

  VkSubpassDescription subpass{};
  subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
  subpass.colorAttachmentCount = 1;
  subpass.pColorAttachments = &colorAttachmentRef;

  VkRenderPassCreateInfo renderPassInfo{};
  renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
  renderPassInfo.attachmentCount = 1;
  renderPassInfo.pAttachments = &colorAttachment;
  renderPassInfo.subpassCount = 1;
  renderPassInfo.pSubpasses = &subpass;

  vkCreateRenderPass(m_rhi->m_device, &renderPassInfo, nullptr, &m_test_pass);
}

void TestPass::CreateVkRenderPipeline() {
  auto device = m_rhi->m_device;
  VkShaderModule vs = VkUtil::RuntimeCreateShaderModule(
      device, "./engine/shaders/test.vert", shaderc_vertex_shader);
  VkShaderModule fs = VkUtil::RuntimeCreateShaderModule(
      device, "./engine/shaders/test.frag", shaderc_fragment_shader);

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

  VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
  vertexInputInfo.sType =
      VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
  vertexInputInfo.vertexBindingDescriptionCount = 0;
  vertexInputInfo.vertexAttributeDescriptionCount = 0;

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
      .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
      .viewportCount = 1,
      .pViewports = &viewPort,
      .scissorCount = 1,
      .pScissors = &m_rhi->m_scissor,
  };

  VkPipelineRasterizationStateCreateInfo rasterizer{};
  rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
  rasterizer.depthClampEnable = VK_FALSE;
  rasterizer.rasterizerDiscardEnable = VK_FALSE;
  rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
  rasterizer.lineWidth = 1.0f;
  rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
  rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
  rasterizer.depthBiasEnable = VK_FALSE;

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

  VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
  pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  pipelineLayoutInfo.setLayoutCount = 0;
  pipelineLayoutInfo.pushConstantRangeCount = 0;

  if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr,
                             &m_test_pipeline_layout) != VK_SUCCESS) {
    throw std::runtime_error("failed to create pipeline layout!");
  }

  VkGraphicsPipelineCreateInfo pipelineInfo{};
  pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
  pipelineInfo.stageCount = 2;
  pipelineInfo.pStages = shaderStages;
  pipelineInfo.pVertexInputState = &vertexInputInfo;
  pipelineInfo.pInputAssemblyState = &inputAssembly;
  pipelineInfo.pViewportState = &viewportState;
  pipelineInfo.pRasterizationState = &rasterizer;
  pipelineInfo.pMultisampleState = &multisampling;
  pipelineInfo.pColorBlendState = &colorBlending;
  pipelineInfo.layout = m_test_pipeline_layout;
  pipelineInfo.renderPass = m_test_pass;
  pipelineInfo.subpass = 0;
  pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

  if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo,
                                nullptr, &m_test_pipeline) != VK_SUCCESS) {
    throw std::runtime_error("failed to create graphics pipeline!");
  }

  vkDestroyShaderModule(device, vs, nullptr);
  vkDestroyShaderModule(device, fs, nullptr);
}

void TestPass::CreateFrameBuffers() {
  m_framebuffers.resize(m_rhi->m_swapchain_images.size());
  uint32_t width = m_rhi->m_swapchain_extent.width;
  uint32_t height = m_rhi->m_swapchain_extent.height;
  for (size_t i = 0; i < m_framebuffers.size(); ++i) {
    VkImageView attachments[] = {m_rhi->m_swapchain_imageviews[i]};

    VkFramebufferCreateInfo framebufferInfo{
        .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
        .renderPass = m_test_pass,
        .attachmentCount = 1,
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