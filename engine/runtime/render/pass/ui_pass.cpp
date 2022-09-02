#include "engine/runtime/render/pass/ui_pass.hpp"

#include "engine/runtime/global/global.hpp"
#include "engine/runtime/render/render_system.hpp"
#include "engine/runtime/render/rhi/vulkan/vk_rhi.hpp"
#include "third_party/imgui/backends/imgui_impl_glfw.h"
#include "third_party/imgui/backends/imgui_impl_vulkan.h"
#include "third_party/imgui/imgui.h"

namespace ShaderStory {

void UIPass::Initialize() {
  InitializeImGUIBackend();
  UploadFonts();
}

void UIPass::RunPass() {
  ImGui_ImplVulkan_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();

  // Insert your ui code here.
  bool t = true;
  ImGui::ShowDemoWindow(&t);

  ImGui::Begin("Recreate Test", &t, ImGuiWindowFlags_MenuBar);
  if (ImGui::Button("Recreate")) {
    g_runtime_global_context.m_render_sys->AddPostFrameCallback(
        []() { g_runtime_global_context.m_render_sys->ReloadPipeline(); });
    ;
  }

  ImGui::End();

  ImGui::Render();

  ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(),
                                  m_rhi->GetCurrentCommandBuffer());
  vkCmdEndRenderPass(m_rhi->GetCurrentCommandBuffer());
}

void UIPass::Dispose() {
  ImGui_ImplVulkan_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();
}

void UIPass::SetVkPass(VkRenderPass pass) { m_pass = pass; }

void UIPass::InitializeImGUIBackend() {
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO();
  (void)io;
  ImGui::StyleColorsDark();

  ImGui_ImplGlfw_InitForVulkan(m_rhi->wd, true);
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