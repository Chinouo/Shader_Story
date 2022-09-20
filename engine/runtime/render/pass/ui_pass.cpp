#include "engine/runtime/render/pass/ui_pass.hpp"

#include <iostream>

#include "engine/runtime/framework/ui_manager.hpp"
#include "engine/runtime/global/global.hpp"
#include "engine/runtime/render/rhi/vulkan/vk_rhi.hpp"
#include "third_party/imgui/backends/imgui_impl_glfw.h"
#include "third_party/imgui/backends/imgui_impl_vulkan.h"
#include "third_party/imgui/imgui.h"

namespace ShaderStory {

UIPass::UIPass() { std::cout << "UIPass created.\n"; }

UIPass::~UIPass() {
  std::cout << "UIPass destory.\n";
  ImGui_ImplVulkan_Shutdown();
}

void UIPass::Initialize() {
  InitializeImGUIBackend();
  UploadFonts();
}

void UIPass::RunPass() {
  g_runtime_global_context.m_ui_manager->RecordUIComponentDrawCommand();

  ImGui_ImplVulkan_NewFrame();

  ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(),
                                  m_rhi->GetCurrentCommandBuffer());
  vkCmdEndRenderPass(m_rhi->GetCurrentCommandBuffer());
}

void UIPass::SetVkPass(VkRenderPass pass) { m_pass = pass; }

void UIPass::InitializeImGUIBackend() {
  ImGui_ImplVulkan_InitInfo init_info = {};
  init_info.Instance = m_rhi->m_instance;
  init_info.PhysicalDevice = m_rhi->m_physical_device;
  init_info.Device = m_rhi->m_device;
  init_info.QueueFamily = m_rhi->m_queue_indices.graphic_family.value();
  init_info.Queue = m_rhi->m_graphic_queue;
  init_info.DescriptorPool = m_rhi->m_descriptor_pool;
  init_info.Subpass = (u_int32_t)0;

  init_info.MinImageCount =
      m_rhi->m_swapchain_details.capabilities.minImageCount;
  init_info.ImageCount = m_rhi->m_swapchain_images.size();
  init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

  // you can create pass for imgui, but here we just use given pass,
  // usually is main_camera pass.
  ASSERT(ImGui_ImplVulkan_Init(&init_info, m_pass));
}

void UIPass::UploadFonts() {
  VkCommandBuffer command_buffer = m_rhi->BeginSingleTimeCommands();

  ImGui_ImplVulkan_CreateFontsTexture(command_buffer);

  m_rhi->EndSingleTimeCommands(command_buffer);

  ImGui_ImplVulkan_DestroyFontUploadObjects();
}

}  // namespace ShaderStory
