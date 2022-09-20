#include "engine/runtime/render/pass/main_camera_pass.hpp"

#include "engine/runtime/render/rhi/vulkan/vk_utils.hpp"

namespace ShaderStory {

MainCameraPass::MainCameraPass() {}

MainCameraPass::~MainCameraPass() {}

void MainCameraPass::Initialze() {
  CreateVkRenderPass();
  CreateDesciptorSetLayout();
  CreateVkRenderPipeline();
  CreateDesciptorSet();
  CreateFrameBuffers();
}

void MainCameraPass::Dispose() {}

void MainCameraPass::RunPass() {
  VkCommandBuffer command_buffer = m_rhi->GetCurrentCommandBuffer();

  // offscreen pass
  {
    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = m_offscreen_pass;
    renderPassInfo.framebuffer =
        m_offscreen_framebuffers[m_rhi->m_current_swapchain_image_index];
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = m_rhi->m_swapchain_extent;

    std::array<VkClearValue, 4> clear_vals;
    clear_vals[0].color = {{1.0f, 0.0f, 1.0f, 1.0f}};
    clear_vals[1].color = {{1.0f, 0.0f, 1.0f, 1.0f}};
    clear_vals[2].color = {{1.0f, 0.0f, 1.0f, 1.0f}};
    clear_vals[3].depthStencil = {1.f, 0};
    renderPassInfo.clearValueCount = clear_vals.size();
    renderPassInfo.pClearValues = clear_vals.data();

    vkCmdBeginRenderPass(command_buffer, &renderPassInfo,
                         VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                      m_offscreen_pipeline);

    u_int32_t cur_frame_idx = m_rhi->GetCurrentFrameIndex();
    size_t offset = m_resources->GetPerframeDataObject().GetOffset();
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

  // composite pass
  {
    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = m_composite_pass;
    renderPassInfo.framebuffer =
        m_composite_framebuffers[m_rhi->m_current_swapchain_image_index];
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = m_rhi->m_swapchain_extent;

    std::array<VkClearValue, 1> clear_vals;
    clear_vals[0].color = {{0.3f, 0.4f, 0.5f, 1.0f}};
    renderPassInfo.clearValueCount = clear_vals.size();
    renderPassInfo.pClearValues = clear_vals.data();

    vkCmdBeginRenderPass(command_buffer, &renderPassInfo,
                         VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                      m_composite_pipeline);

    u_int32_t cur_frame_idx = m_rhi->GetCurrentFrameIndex();
    size_t offset = m_resources->GetPerframeDataObject().GetOffset();
    u_int32_t dy_offsets = cur_frame_idx * offset;

    std::array<VkDescriptorSet, 3> bound_sets;
    bound_sets[0] = m_composite_dybuffer_set;
    bound_sets[1] = m_composite_gbuffer_sets[cur_frame_idx];
    bound_sets[2] = m_cascade_shadowmap_sets[cur_frame_idx];

    vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                            m_composite_pipeline_layout, 0, bound_sets.size(),
                            bound_sets.data(), 1, &dy_offsets);

    vkCmdDraw(command_buffer, 6, 1, 0, 0);
  }
}

void MainCameraPass::CreateVkRenderPass() {
  // offscreen
  {
    std::array<VkAttachmentDescription, 4> attachments;
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

    // depth
    attachments[3].format = m_resources->GetGBufferResources().gDepthFmt;
    attachments[3].samples = VK_SAMPLE_COUNT_1_BIT;
    attachments[3].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachments[3].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachments[3].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachments[3].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[3].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachments[3].finalLayout =
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
    attachments[3].flags = 0;

    std::array<VkAttachmentReference, 3> color_refs;
    color_refs[0].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    color_refs[0].attachment = 0;
    color_refs[1].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    color_refs[1].attachment = 1;
    color_refs[2].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    color_refs[2].attachment = 2;

    VkAttachmentReference depth_ref;
    depth_ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    depth_ref.attachment = 3;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = color_refs.size();
    subpass.pColorAttachments = color_refs.data();
    subpass.inputAttachmentCount = 0;
    subpass.pDepthStencilAttachment = &depth_ref;

    std::array<VkSubpassDependency, 2> dependencies;
    dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[0].dstSubpass = 0;
    dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    dependencies[0].dstStageMask =
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
                                    VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    dependencies[1].srcSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[1].dstSubpass = 0;
    dependencies[1].srcStageMask =
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
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

  // composite
  {
    std::array<VkAttachmentDescription, 1> attachments;
    // color
    attachments[0].format = m_rhi->m_swapchain_format;
    attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
    attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachments[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    attachments[0].flags = 0;

    VkAttachmentReference color_ref;
    color_ref.attachment = 0;
    color_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &color_ref;
    subpass.inputAttachmentCount = 0;
    subpass.pDepthStencilAttachment = nullptr;

    // ??
    std::array<VkSubpassDependency, 1> dependencies;
    dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[0].dstSubpass = 0;
    dependencies[0].srcStageMask =
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[0].dstStageMask =
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[0].srcAccessMask = 0;
    dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = attachments.size();
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = dependencies.size();
    renderPassInfo.pDependencies = dependencies.data();

    VK_CHECK(vkCreateRenderPass(m_rhi->m_device, &renderPassInfo, nullptr,
                                &m_composite_pass),
             "Create renderpass failed.");
  }
}

void MainCameraPass::CreateDesciptorSetLayout() {
  // offscreen
  {
    std::array<VkDescriptorSetLayoutBinding, 2> bindings;
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

    VkDescriptorSetLayoutCreateInfo info{
        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
    info.bindingCount = bindings.size();
    info.pBindings = bindings.data();

    VK_CHECK(vkCreateDescriptorSetLayout(m_rhi->m_device, &info, nullptr,
                                         &m_offscreen_set_layout),
             "Failed to create offscreen desp set.");
  }

  // composite
  {
    VkDescriptorSetLayoutCreateInfo info{
        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};

    std::array<VkDescriptorSetLayoutBinding, 1> bindings;
    // perframe data bindings.
    bindings[0].binding = 0;
    bindings[0].descriptorCount = 1;
    bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    bindings[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    info.bindingCount = bindings.size();
    info.pBindings = bindings.data();

    VK_CHECK(vkCreateDescriptorSetLayout(m_rhi->m_device, &info, nullptr,
                                         &m_composite_dybuffer_set_layout),
             "Failed to create offscreen desp set layout.");

    std::array<VkDescriptorSetLayoutBinding, 4> gbuffer_bindings;
    // g-position.
    gbuffer_bindings[0].binding = 0;
    gbuffer_bindings[0].descriptorCount = 1;
    gbuffer_bindings[0].descriptorType =
        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    gbuffer_bindings[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    gbuffer_bindings[0].pImmutableSamplers = nullptr;

    // g-normal
    gbuffer_bindings[1].binding = 1;
    gbuffer_bindings[1].descriptorCount = 1;
    gbuffer_bindings[1].descriptorType =
        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    gbuffer_bindings[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    gbuffer_bindings[1].pImmutableSamplers = nullptr;

    // g-albedo
    gbuffer_bindings[2].binding = 2;
    gbuffer_bindings[2].descriptorCount = 1;
    gbuffer_bindings[2].descriptorType =
        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    gbuffer_bindings[2].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    gbuffer_bindings[2].pImmutableSamplers = nullptr;

    // g-depth
    gbuffer_bindings[3].binding = 3;
    gbuffer_bindings[3].descriptorCount = 1;
    gbuffer_bindings[3].descriptorType =
        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    gbuffer_bindings[3].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    gbuffer_bindings[3].pImmutableSamplers = nullptr;

    info.bindingCount = gbuffer_bindings.size();
    info.pBindings = gbuffer_bindings.data();

    VK_CHECK(vkCreateDescriptorSetLayout(m_rhi->m_device, &info, nullptr,
                                         &m_composite_gbuffer_set_layout),
             "Failed to create composite gbuffer desp set layout.");
  }

  // cascade shadowmap
  {
    VkDescriptorSetLayoutBinding cascade_shadowmap_binding{};
    cascade_shadowmap_binding.binding = 0;
    cascade_shadowmap_binding.descriptorCount = 1;
    cascade_shadowmap_binding.descriptorType =
        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    cascade_shadowmap_binding.pImmutableSamplers = nullptr;
    cascade_shadowmap_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutCreateInfo info{
        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
    info.bindingCount = 1;
    info.pBindings = &cascade_shadowmap_binding;

    VK_CHECK(vkCreateDescriptorSetLayout(m_rhi->m_device, &info, nullptr,
                                         &m_cascade_shadowmaps_set_layout),
             "Failed to create composite gbuffer desp set layout.");
  }
}

void MainCameraPass::CreateVkRenderPipeline() {
  auto device = m_rhi->m_device;
  // offscreen
  {
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

    // we have 3 color attachments.
    std::array<VkPipelineColorBlendAttachmentState, 3> blend_attachment_states;

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

  // composite
  {
    // set up shader
    VkShaderModule vs = VkUtil::RuntimeCreateShaderModule(
        device, "./shaders/composite.vert", shaderc_vertex_shader);
    VkShaderModule fs = VkUtil::RuntimeCreateShaderModule(
        device, "./shaders/composite.frag", shaderc_fragment_shader);

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
    std::array<VkDescriptorSetLayout, 3> composite_setlayouts;
    composite_setlayouts[0] = m_composite_dybuffer_set_layout;
    composite_setlayouts[1] = m_composite_gbuffer_set_layout;
    composite_setlayouts[2] = m_cascade_shadowmaps_set_layout;

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.pSetLayouts = composite_setlayouts.data();
    pipelineLayoutInfo.setLayoutCount = composite_setlayouts.size();
    pipelineLayoutInfo.pushConstantRangeCount = 0;

    VK_CHECK(vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr,
                                    &m_composite_pipeline_layout),
             "Failed to create composite pipeline layout.");

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
    pipelineInfo.layout = m_composite_pipeline_layout;
    pipelineInfo.renderPass = m_composite_pass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

    VK_CHECK(vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo,
                                       nullptr, &m_composite_pipeline),
             "Failed to create composite pipeline!");

    vkDestroyShaderModule(device, vs, nullptr);
    vkDestroyShaderModule(device, fs, nullptr);
  }
}

void MainCameraPass::CreateDesciptorSet() {
  // offscreen
  {
    // allocate set.
    VkDescriptorSetAllocateInfo info{
        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
    info.descriptorPool = m_rhi->m_descriptor_pool;
    info.descriptorSetCount = 1;
    info.pSetLayouts = &m_offscreen_set_layout;

    VK_CHECK(vkAllocateDescriptorSets(m_rhi->m_device, &info, &m_offscreen_set),
             "Failed to create desp set.");

    // perframe buffer set.
    VkDescriptorBufferInfo perframe_data_buf_info{};
    perframe_data_buf_info.offset = 0;
    perframe_data_buf_info.range = sizeof(PerframeData);
    perframe_data_buf_info.buffer =
        m_resources->GetPerframeDataObject().perframe_data_buffer;

    // mesh texture set
    VkDescriptorImageInfo terrain_texture_info{};
    terrain_texture_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    terrain_texture_info.imageView =
        m_resources->GetTerrainTextureObject().texture_image_view;
    terrain_texture_info.sampler = m_resources->GetTerrainSampler();

    std::array<VkWriteDescriptorSet, 2> writers;
    // perframe data writer
    writers[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writers[0].pNext = nullptr;
    writers[0].dstSet = m_offscreen_set;
    writers[0].dstBinding = 0;
    writers[0].dstArrayElement = 0;
    writers[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    writers[0].descriptorCount = 1;
    writers[0].pBufferInfo = &perframe_data_buf_info;

    writers[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writers[1].pNext = nullptr;
    writers[1].dstSet = m_offscreen_set;
    writers[1].dstBinding = 1;
    writers[1].dstArrayElement = 0;
    writers[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    writers[1].descriptorCount = 1;
    writers[1].pImageInfo = &terrain_texture_info;

    vkUpdateDescriptorSets(m_rhi->m_device, writers.size(), writers.data(), 0,
                           nullptr);
  }

  // composite
  {
    VkDescriptorSetAllocateInfo info{
        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};

    // dynamic set alloc and update.
    info.descriptorPool = m_rhi->m_descriptor_pool;
    info.descriptorSetCount = 1;
    info.pSetLayouts = &m_composite_dybuffer_set_layout;

    VK_CHECK(vkAllocateDescriptorSets(m_rhi->m_device, &info,
                                      &m_composite_dybuffer_set),
             "Failed to create desp set.");

    // perframe buffer set.
    VkDescriptorBufferInfo perframe_data_buf_info{};
    perframe_data_buf_info.offset = 0;
    perframe_data_buf_info.range = sizeof(PerframeData);
    perframe_data_buf_info.buffer =
        m_resources->GetPerframeDataObject().perframe_data_buffer;

    std::array<VkWriteDescriptorSet, 1> dyset_writers;
    // perframe data writer
    dyset_writers[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    dyset_writers[0].pNext = nullptr;
    dyset_writers[0].dstSet = m_composite_dybuffer_set;
    dyset_writers[0].dstBinding = 0;
    dyset_writers[0].dstArrayElement = 0;
    dyset_writers[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    dyset_writers[0].descriptorCount = 1;
    dyset_writers[0].pBufferInfo = &perframe_data_buf_info;

    vkUpdateDescriptorSets(m_rhi->m_device, dyset_writers.size(),
                           dyset_writers.data(), 0, nullptr);

    // gbuffer set updata and alloc
    m_composite_gbuffer_sets.resize(MAX_FRAMES_IN_FLIGHT);
    std::vector<VkDescriptorSetLayout> gbuffer_set_layouts(
        MAX_FRAMES_IN_FLIGHT, m_composite_gbuffer_set_layout);
    info.descriptorPool = m_rhi->m_descriptor_pool;
    info.descriptorSetCount = gbuffer_set_layouts.size();
    info.pSetLayouts = gbuffer_set_layouts.data();

    VK_CHECK(vkAllocateDescriptorSets(m_rhi->m_device, &info,
                                      m_composite_gbuffer_sets.data()),
             "Failed to create desp set.");

    VkDescriptorImageInfo gPositionInfo{};
    VkDescriptorImageInfo gNormalInfo{};
    VkDescriptorImageInfo gAlbedoInfo{};
    VkDescriptorImageInfo gDepthInfo{};
    // use same sampler.
    gPositionInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    gPositionInfo.sampler = m_resources->GetGBufferResources().gBufferSampler;
    gNormalInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    gNormalInfo.sampler = gPositionInfo.sampler;
    gAlbedoInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    gAlbedoInfo.sampler = gPositionInfo.sampler;
    // note: different layout!!!
    gDepthInfo.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
    gDepthInfo.sampler = m_resources->GetSunResourceObject().shadowmap_sampler;

    std::array<VkWriteDescriptorSet, 4> gbuffer_writes;
    // position-ws
    gbuffer_writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    gbuffer_writes[0].pNext = nullptr;
    gbuffer_writes[0].dstBinding = 0;
    gbuffer_writes[0].dstArrayElement = 0;
    gbuffer_writes[0].descriptorType =
        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    gbuffer_writes[0].descriptorCount = 1;
    gbuffer_writes[0].pImageInfo = &gPositionInfo;

    // normal-ws
    gbuffer_writes[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    gbuffer_writes[1].pNext = nullptr;
    gbuffer_writes[1].dstBinding = 1;
    gbuffer_writes[1].dstArrayElement = 0;
    gbuffer_writes[1].descriptorType =
        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    gbuffer_writes[1].descriptorCount = 1;
    gbuffer_writes[1].pImageInfo = &gNormalInfo;

    // albedo
    gbuffer_writes[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    gbuffer_writes[2].pNext = nullptr;
    gbuffer_writes[2].dstBinding = 2;
    gbuffer_writes[2].dstArrayElement = 0;
    gbuffer_writes[2].descriptorType =
        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    gbuffer_writes[2].descriptorCount = 1;
    gbuffer_writes[2].pImageInfo = &gAlbedoInfo;

    // depth
    gbuffer_writes[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    gbuffer_writes[3].pNext = nullptr;
    gbuffer_writes[3].dstBinding = 3;
    gbuffer_writes[3].dstArrayElement = 0;
    gbuffer_writes[3].descriptorType =
        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    gbuffer_writes[3].descriptorCount = 1;
    gbuffer_writes[3].pImageInfo = &gDepthInfo;

    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
      // position-ws
      gPositionInfo.imageView = m_resources->GetGBufferObject(i).gPositionView;
      gbuffer_writes[0].dstSet = m_composite_gbuffer_sets[i];

      // normal-ws
      gNormalInfo.imageView = m_resources->GetGBufferObject(i).gNormalView;
      gbuffer_writes[1].dstSet = m_composite_gbuffer_sets[i];

      // albedo
      gAlbedoInfo.imageView = m_resources->GetGBufferObject(i).gColorView;
      gbuffer_writes[2].dstSet = m_composite_gbuffer_sets[i];

      // depth
      gDepthInfo.imageView = m_resources->GetGBufferObject(i).gDepthView;
      gbuffer_writes[3].dstSet = m_composite_gbuffer_sets[i];

      vkUpdateDescriptorSets(m_rhi->m_device, gbuffer_writes.size(),
                             gbuffer_writes.data(), 0, nullptr);
    }
  }

  // cascade shadowmap
  {
    VkDescriptorSetAllocateInfo info{
        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};

    m_cascade_shadowmap_sets.resize(MAX_FRAMES_IN_FLIGHT);
    std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT,
                                               m_cascade_shadowmaps_set_layout);

    info.descriptorPool = m_rhi->m_descriptor_pool;
    info.descriptorSetCount = layouts.size();
    info.pSetLayouts = layouts.data();

    VK_CHECK(vkAllocateDescriptorSets(m_rhi->m_device, &info,
                                      m_cascade_shadowmap_sets.data()),
             "Failed to create cascade shadowmap desp set.");

    VkDescriptorImageInfo cascade_shadowmap_info{};
    cascade_shadowmap_info.imageLayout =
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
    cascade_shadowmap_info.sampler =
        m_resources->GetSunResourceObject().shadowmap_sampler;

    VkWriteDescriptorSet cascade_shadowmap_write{};
    cascade_shadowmap_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    cascade_shadowmap_write.pNext = nullptr;
    cascade_shadowmap_write.dstBinding = 0;
    cascade_shadowmap_write.dstArrayElement = 0;
    cascade_shadowmap_write.descriptorType =
        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    cascade_shadowmap_write.descriptorCount = 1;
    cascade_shadowmap_write.pImageInfo = &cascade_shadowmap_info;

    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
      cascade_shadowmap_info.imageView =
          m_resources->GetSunResourceObject().cascade_shadowmap_views[i];
      cascade_shadowmap_write.dstSet = m_cascade_shadowmap_sets[i];
      vkUpdateDescriptorSets(m_rhi->m_device, 1, &cascade_shadowmap_write, 0,
                             nullptr);
    }
  }
}

void MainCameraPass::CreateFrameBuffers() {
  // offscreen framebuffer
  {
    m_offscreen_framebuffers.resize(MAX_FRAMES_IN_FLIGHT);

    VkFramebufferCreateInfo fbf_info = {};
    fbf_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    fbf_info.pNext = nullptr;
    fbf_info.renderPass = m_offscreen_pass;
    fbf_info.width = m_rhi->m_swapchain_extent.width;
    fbf_info.height = m_rhi->m_swapchain_extent.height;
    fbf_info.layers = 1;

    for (int i = 0; i < m_offscreen_framebuffers.size(); ++i) {
      std::array<VkImageView, 4> attachments;
      attachments[0] = m_resources->GetGBufferObject(i).gPositionView;
      attachments[1] = m_resources->GetGBufferObject(i).gNormalView;
      attachments[2] = m_resources->GetGBufferObject(i).gColorView;
      attachments[3] = m_resources->GetGBufferObject(i).gDepthView;

      fbf_info.attachmentCount = attachments.size();
      fbf_info.pAttachments = attachments.data();

      VK_CHECK(vkCreateFramebuffer(m_rhi->m_device, &fbf_info, nullptr,
                                   &m_offscreen_framebuffers[i]),
               "Failed to create framebuffer!");
    }
  }

  // composite framebuffer
  {
    m_composite_framebuffers.resize(MAX_FRAMES_IN_FLIGHT);
    VkFramebufferCreateInfo fbf_info = {};
    fbf_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    fbf_info.pNext = nullptr;
    fbf_info.renderPass = m_composite_pass;
    fbf_info.width = m_rhi->m_swapchain_extent.width;
    fbf_info.height = m_rhi->m_swapchain_extent.height;
    fbf_info.layers = 1;

    for (int i = 0; i < m_offscreen_framebuffers.size(); ++i) {
      std::array<VkImageView, 1> attachments;
      attachments[0] = m_rhi->m_swapchain_imageviews[i];

      fbf_info.attachmentCount = attachments.size();
      fbf_info.pAttachments = attachments.data();

      VK_CHECK(vkCreateFramebuffer(m_rhi->m_device, &fbf_info, nullptr,
                                   &m_composite_framebuffers[i]),
               "Failed to create composite framebuffer!");
    }
  }
}

}  // namespace ShaderStory