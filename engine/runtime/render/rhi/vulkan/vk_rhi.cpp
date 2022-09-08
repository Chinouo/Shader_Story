
#include "engine/runtime/render/rhi/vulkan/vk_rhi.hpp"

#include <exception>
#include <iostream>
#include <set>

#include "engine/runtime/render/rhi/vulkan/vk_utils.hpp"

namespace ShaderStory {

namespace RHI {

VKAPI_ATTR VkBool32 VKAPI_CALL
VKRHI::DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                     VkDebugUtilsMessageTypeFlagsEXT messageType,
                     const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
                     void *pUserData) {
  if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
    std::cerr << "Validation layer: " << pCallbackData->pMessage << std::endl;
  }
  return VK_FALSE;
}

QueueFamilyIndices VKRHI::FindQueueFamilies(VkPhysicalDevice pd,
                                            VkSurfaceKHR surface) {
  QueueFamilyIndices indices;
  uint32_t queue_family_count = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(pd, &queue_family_count, nullptr);
  std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
  vkGetPhysicalDeviceQueueFamilyProperties(pd, &queue_family_count,
                                           queue_families.data());

  int i = 0;
  for (const auto &queue_family : queue_families) {
    // if support graphics command queue
    if (queue_family.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
      indices.graphic_family = i;
    }

    VkBool32 is_present_support = false;
    // if support surface presentation
    vkGetPhysicalDeviceSurfaceSupportKHR(pd, i, surface, &is_present_support);
    if (is_present_support) {
      indices.present_family = i;
    }

    if (indices.IsComplete()) {
      break;
    }
    i++;
  }
  return indices;
}

VKRHI::VKRHI() {}

VKRHI::~VKRHI() { Destory(); }

void VKRHI::Initialize(GLFWwindow *wd) {
  this->wd = wd;
  CreateInstance();
  CreateDebugMessenger();
  CreateWindowSurface();
  InitialzePhysicalDevice();
  CreateLoginDevice();
  CreateCommandPool();
  CreateCommandBuffers();
  CreateDescriptorPool();
  CreateSyncPrimitives();

  CreateSwapchain();
  CreateSwapchainImageViews();
  CreateFramebufferImageAndView();
  CreateAssetAllocator();
}

void VKRHI::Destory() {
  vmaDestroyAllocator(m_vma_allocator);

  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
    if (m_command_pools[i])
      vkDestroyCommandPool(m_device, m_command_pools[i], nullptr);

    if (m_image_render_finish_sema[i])
      vkDestroySemaphore(m_device, m_image_render_finish_sema[i], nullptr);

    if (m_image_present_finish_sema[i])
      vkDestroySemaphore(m_device, m_image_present_finish_sema[i], nullptr);

    if (m_is_frame_in_flight[i])
      vkDestroyFence(m_device, m_is_frame_in_flight[i], nullptr);

    if (m_swapchain_imageviews[i])
      vkDestroyImageView(m_device, m_swapchain_imageviews[i], nullptr);
  }

  if (m_persist_command_pool)
    vkDestroyCommandPool(m_device, m_persist_command_pool, nullptr);

  if (m_swapchain) vkDestroySwapchainKHR(m_device, m_swapchain, nullptr);

  if (m_descriptor_pool)
    vkDestroyDescriptorPool(m_device, m_descriptor_pool, nullptr);

  if (m_debug_messenger) DestoryDebugMessenger();

  // only when destory device, vaildation layer will dump unrelease resources.
  if (m_device) vkDestroyDevice(m_device, nullptr);
  if (m_surface) vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
  if (m_instance) vkDestroyInstance(m_instance, nullptr);
}

void VKRHI::CreateInstance() {
  VkApplicationInfo app_info{};
  app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
  app_info.applicationVersion = VK_MAKE_VERSION(0, 0, 1);
  app_info.pEngineName = "ShaderStory";
  app_info.engineVersion = VK_MAKE_VERSION(0, 0, 1);
  app_info.apiVersion = VK_API_VERSION_1_3;

  std::vector<const char *> instance_extensions{
      VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME,
      VK_EXT_DEBUG_UTILS_EXTENSION_NAME,  // for debug.
  };

  std::vector<const char *> layers{
      "VK_LAYER_KHRONOS_validation",  // for debug.
  };

  uint32_t glfwRequiredExtCount;
  const char **glfwRequiredExts;
  glfwRequiredExts = glfwGetRequiredInstanceExtensions(&glfwRequiredExtCount);

  std::vector<const char *> glfwReExts(glfwRequiredExts,
                                       glfwRequiredExts + glfwRequiredExtCount);

  // may duplicated.
  instance_extensions.insert(instance_extensions.cbegin(), glfwReExts.cbegin(),
                             glfwReExts.cend());

  VkInstanceCreateInfo info{};
  info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
  info.flags = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR,
  info.pApplicationInfo = &app_info;
  info.enabledLayerCount = static_cast<uint32_t>(layers.size());
  info.ppEnabledLayerNames = layers.data(),
  info.enabledExtensionCount = instance_extensions.size();
  info.ppEnabledExtensionNames = instance_extensions.data();

  VK_CHECK(vkCreateInstance(&info, nullptr, &m_instance),
           "Failed to create instance.");
}

void VKRHI::CreateDebugMessenger() {
  VkDebugUtilsMessengerCreateInfoEXT info{};
  info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;

  info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                         VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                         VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;

  info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                     VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                     VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
  info.pfnUserCallback = DebugCallback;
  info.pUserData = nullptr;

  auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
      m_instance, "vkCreateDebugUtilsMessengerEXT");

  ASSERT(func);

  VK_CHECK(func(m_instance, &info, nullptr, &m_debug_messenger),
           "Failed to create debug messenger.")
}

void VKRHI::DestoryDebugMessenger() {
  auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
      m_instance, "vkDestroyDebugUtilsMessengerEXT");
  ASSERT(func);
  func(m_instance, m_debug_messenger, nullptr);
}

void VKRHI::CreateWindowSurface() {
  VK_CHECK(glfwCreateWindowSurface(m_instance, wd, nullptr, &m_surface),
           "Failed to create surface.");
}

void VKRHI::InitialzePhysicalDevice() {
  uint32_t device_count;
  vkEnumeratePhysicalDevices(m_instance, &device_count, nullptr);
  std::vector<VkPhysicalDevice> physical_devices(device_count);
  ASSERT(device_count > 0 && "No device detected.");
  vkEnumeratePhysicalDevices(m_instance, &device_count,
                             physical_devices.data());

  // you may need pick up by yourselves
  m_physical_device = physical_devices.front();
  vkGetPhysicalDeviceProperties(m_physical_device, &m_pd_property);
  vkGetPhysicalDeviceFeatures(m_physical_device, &m_pd_features);
  vkGetPhysicalDeviceMemoryProperties(m_physical_device, &m_pd_memory_property);
}

void VKRHI::CreateLoginDevice() {
  m_queue_indices = FindQueueFamilies(m_physical_device, m_surface);
  std::vector<VkDeviceQueueCreateInfo> queue_create_infos;
  std::set<uint32_t> queue_families = {
      m_queue_indices.graphic_family.value(),
      m_queue_indices.present_family.value(),
  };

  float queue_priority = 1.0f;
  for (uint32_t queue_family : queue_families)  // for every queue family
  {
    // queue create info
    VkDeviceQueueCreateInfo queue_create_info{};
    queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_create_info.queueFamilyIndex = queue_family;
    queue_create_info.queueCount = 1;
    queue_create_info.pQueuePriorities = &queue_priority;
    queue_create_infos.push_back(queue_create_info);
  }
  // TODO: check support.
  VkPhysicalDeviceFeatures enabled_feature{};
  enabled_feature.samplerAnisotropy = VK_TRUE;
  enabled_feature.fragmentStoresAndAtomics = VK_TRUE;
  enabled_feature.independentBlend = VK_TRUE;

  // TODO: check support.
  std::vector<const char *> device_extensions{
      VK_KHR_SWAPCHAIN_EXTENSION_NAME,
      "VK_KHR_portability_subset",
  };

  VkDeviceCreateInfo info{};
  info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  info.pQueueCreateInfos = queue_create_infos.data();
  info.queueCreateInfoCount = queue_create_infos.size();
  info.pEnabledFeatures = &enabled_feature;
  info.enabledExtensionCount = device_extensions.size();
  info.ppEnabledExtensionNames = device_extensions.data();
  info.enabledLayerCount = 0;

  // check feature available.
  VK_CHECK(vkCreateDevice(m_physical_device, &info, nullptr, &m_device),
           "Failed to create logic device.");

  vkGetDeviceQueue(m_device, m_queue_indices.graphic_family.value(), 0,
                   &m_graphic_queue);
  vkGetDeviceQueue(m_device, m_queue_indices.present_family.value(), 0,
                   &m_present_queue);
}

void VKRHI::CreateCommandPool() {
  // persisit cmd pool
  VkCommandPoolCreateInfo g_info{};
  g_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  g_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
  g_info.queueFamilyIndex = m_queue_indices.graphic_family.value();
  g_info.pNext = nullptr;

  VK_CHECK(
      vkCreateCommandPool(m_device, &g_info, nullptr, &m_persist_command_pool),
      "Create graphic command pool failed.");

  // other pools
  VkCommandPoolCreateInfo o_info{};
  o_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  o_info.pNext = nullptr;
  o_info.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
  o_info.queueFamilyIndex = m_queue_indices.graphic_family.value();
  for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
    VK_CHECK(
        vkCreateCommandPool(m_device, &o_info, nullptr, &m_command_pools[i]),
        "Create other command pool failed.");
  }
}

void VKRHI::CreateCommandBuffers() {
  VkCommandBufferAllocateInfo info{};
  info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  info.commandBufferCount = 1U;

  for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
    info.commandPool = m_command_pools[i];

    VK_CHECK(vkAllocateCommandBuffers(m_device, &info, &m_command_buffers[i]),
             "Falied to allocate command buffer.");
  }
}

void VKRHI::CreateDescriptorPool() {
  VkDescriptorPoolSize pool_sizes[] = {
      {VK_DESCRIPTOR_TYPE_SAMPLER, 1000},
      {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000},
      {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000},
      {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000},
      {VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000},
      {VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000},
      {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000},
      {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000},
      {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000},
      {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000},
      {VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000}};

  auto sz = sizeof(pool_sizes) / sizeof(VkDescriptorPoolSize);
  VkDescriptorPoolCreateInfo info = {};
  info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
  info.maxSets = 1000 * sz;
  info.poolSizeCount = static_cast<uint32_t>(sz);
  info.pPoolSizes = pool_sizes;

  VK_CHECK(vkCreateDescriptorPool(m_device, &info, nullptr, &m_descriptor_pool),
           "Failed to create descriptorPool.");
}

void VKRHI::CreateSyncPrimitives() {
  VkSemaphoreCreateInfo sem_info{};
  sem_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
  VkFenceCreateInfo fence_info{};
  fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
  fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

  for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    VK_CHECK(vkCreateSemaphore(m_device, &sem_info, nullptr,
                               &m_image_render_finish_sema[i]),
             "Failed to create semaphroe.");

    VK_CHECK(vkCreateSemaphore(m_device, &sem_info, nullptr,
                               &m_image_present_finish_sema[i]),
             "Failed to create semaphroe.");

    VK_CHECK(
        vkCreateFence(m_device, &fence_info, nullptr, &m_is_frame_in_flight[i]),
        "Failed to create fence.");
  }
}

SwapChainSupportDetails VKRHI::QuerySwapChainSupport() {
  SwapChainSupportDetails details{};
  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_physical_device, m_surface,
                                            &details.capabilities);

  uint32_t format_count;
  vkGetPhysicalDeviceSurfaceFormatsKHR(m_physical_device, m_surface,
                                       &format_count, nullptr);

  ASSERT(format_count != 0 && "No surface format found.");
  details.formats.resize(format_count);
  vkGetPhysicalDeviceSurfaceFormatsKHR(m_physical_device, m_surface,
                                       &format_count, details.formats.data());
  uint32_t presentmode_count;
  vkGetPhysicalDeviceSurfacePresentModesKHR(m_physical_device, m_surface,
                                            &presentmode_count, nullptr);
  ASSERT(format_count != 0 && "No present mode found.");

  details.presentModes.resize(presentmode_count);
  vkGetPhysicalDeviceSurfacePresentModesKHR(m_physical_device, m_surface,
                                            &presentmode_count,
                                            details.presentModes.data());

  return details;
}

VkExtent2D VKRHI::ChooseSwapchainExtent(
    const VkSurfaceCapabilitiesKHR &capabilities) {
  if (capabilities.currentExtent.width != UINT32_MAX) {
    return capabilities.currentExtent;
  } else {
    int width, height;
    glfwGetFramebufferSize(wd, &width, &height);

    VkExtent2D actualExtent = {static_cast<uint32_t>(width),
                               static_cast<uint32_t>(height)};

    actualExtent.width =
        std::clamp(actualExtent.width, capabilities.minImageExtent.width,
                   capabilities.maxImageExtent.width);
    actualExtent.height =
        std::clamp(actualExtent.height, capabilities.minImageExtent.height,
                   capabilities.maxImageExtent.height);

    return actualExtent;
  }
}

VkPresentModeKHR VKRHI::ChoosePresentMode(
    const std::vector<VkPresentModeKHR> &available_modes) {
  for (VkPresentModeKHR present_mode : available_modes) {
    if (VK_PRESENT_MODE_MAILBOX_KHR == present_mode) {
      return VK_PRESENT_MODE_MAILBOX_KHR;
    }
  }

  return VK_PRESENT_MODE_FIFO_KHR;
}

VkSurfaceFormatKHR VKRHI::ChooseSurfaceFormat(
    const std::vector<VkSurfaceFormatKHR> &available_formats) {
  for (const auto &surface_format : available_formats) {
    if (surface_format.format == VK_FORMAT_B8G8R8A8_SRGB &&
        surface_format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
      return surface_format;
    }
  }
  return available_formats.front();
}

void VKRHI::CreateSwapchain() {
  m_swapchain_details = QuerySwapChainSupport();

  VkSurfaceFormatKHR fmt = ChooseSurfaceFormat(m_swapchain_details.formats);
  VkPresentModeKHR present_mode =
      ChoosePresentMode(m_swapchain_details.presentModes);

  VkExtent2D extent = ChooseSwapchainExtent(m_swapchain_details.capabilities);

  uint32_t image_count = m_swapchain_details.capabilities.minImageCount + 1;
  if (m_swapchain_details.capabilities.maxImageCount > 0 &&
      image_count > m_swapchain_details.capabilities.maxImageCount) {
    image_count = m_swapchain_details.capabilities.maxImageCount;
  }

  VkSwapchainCreateInfoKHR info{};
  info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  info.surface = m_surface;

  info.minImageCount = image_count;
  info.imageFormat = fmt.format;
  info.imageColorSpace = fmt.colorSpace;
  info.imageExtent = extent;
  info.imageArrayLayers = 1;
  info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

  uint32_t queueFamilyIndices[] = {
      m_queue_indices.graphic_family.value(),
      m_queue_indices.present_family.value(),
  };

  if (m_queue_indices.graphic_family != m_queue_indices.present_family) {
    info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
    info.queueFamilyIndexCount = 2;
    info.pQueueFamilyIndices = queueFamilyIndices;
  } else {
    info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
  }

  info.preTransform = m_swapchain_details.capabilities.currentTransform;
  info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
  info.presentMode = present_mode;
  info.clipped = VK_TRUE;

  info.oldSwapchain = VK_NULL_HANDLE;

  VK_CHECK(vkCreateSwapchainKHR(m_device, &info, nullptr, &m_swapchain),
           "Failed to create swapchain.");

  vkGetSwapchainImagesKHR(m_device, m_swapchain, &image_count, nullptr);
  m_swapchain_images.resize(image_count);
  m_swapchain_imageviews.resize(image_count);
  vkGetSwapchainImagesKHR(m_device, m_swapchain, &image_count,
                          m_swapchain_images.data());

  m_swapchain_format = fmt.format;
  m_swapchain_extent = extent;

  m_scissor = {{0, 0}, {m_swapchain_extent.width, m_swapchain_extent.height}};
}

void VKRHI::CreateSwapchainImageViews() {
  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
    m_swapchain_imageviews[i] = VkUtil::CreateImageView(
        m_device, m_swapchain_images[i], m_swapchain_format,
        VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_VIEW_TYPE_2D, 1, 1);
  }
}

void VKRHI::CreateFramebufferImageAndView() {}

void VKRHI::CreateAssetAllocator() {
  VmaVulkanFunctions vulkanFunctions = {};
  vulkanFunctions.vkGetInstanceProcAddr = &vkGetInstanceProcAddr;
  vulkanFunctions.vkGetDeviceProcAddr = &vkGetDeviceProcAddr;

  VmaAllocatorCreateInfo allocatorCreateInfo = {};
  // note: check create instance function
  // version 1_3 current not support macOS
  // from vulkan doc, different version within a instance is fine.
  allocatorCreateInfo.vulkanApiVersion = VK_API_VERSION_1_2;
  allocatorCreateInfo.physicalDevice = m_physical_device;
  allocatorCreateInfo.device = m_device;
  allocatorCreateInfo.instance = m_instance;
  allocatorCreateInfo.pVulkanFunctions = &vulkanFunctions;

  vmaCreateAllocator(&allocatorCreateInfo, &m_vma_allocator);
}

// util func
VkCommandBuffer VKRHI::BeginSingleTimeCommands() {
  VkCommandBuffer command_buffer;
  VkCommandBufferAllocateInfo info{};
  info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  info.commandPool = m_persist_command_pool;
  info.commandBufferCount = 1;

  VK_CHECK(vkAllocateCommandBuffers(m_device, &info, &command_buffer),
           "Create single time command buffer failed.");

  VkCommandBufferBeginInfo begin_info{};
  begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

  VK_CHECK(vkBeginCommandBuffer(command_buffer, &begin_info),
           "Failed to begin single time command buffer.");

  return command_buffer;
}

void VKRHI::EndSingleTimeCommands(VkCommandBuffer command_buffer) {
  vkEndCommandBuffer(command_buffer);

  VkSubmitInfo submit_info{};
  submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submit_info.commandBufferCount = 1;
  submit_info.pCommandBuffers = &command_buffer;

  vkQueueSubmit(m_graphic_queue, 1, &submit_info, VK_NULL_HANDLE);
  vkQueueWaitIdle(m_graphic_queue);

  vkFreeCommandBuffers(m_device, m_persist_command_pool, 1, &command_buffer);
}

// render func
void VKRHI::WarmUpBeforePass() {
  // we only have a fixed-size swapchain.
  VK_CHECK(
      vkAcquireNextImageKHR(m_device, m_swapchain, UINT64_MAX,
                            m_image_render_finish_sema[m_current_frame_index],
                            VK_NULL_HANDLE, &m_current_swapchain_image_index),
      "Exception occured when getting swapchain image.");

  // begin command buffer
  VkCommandBufferBeginInfo command_buffer_begin_info{};
  command_buffer_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  command_buffer_begin_info.flags = 0;
  command_buffer_begin_info.pInheritanceInfo = nullptr;

  VK_CHECK(vkBeginCommandBuffer(m_command_buffers[m_current_frame_index],
                                &command_buffer_begin_info),
           "Failed to begin command buffer before logic pass.");
}

void VKRHI::WaitForFence() {
  VK_CHECK(
      vkWaitForFences(m_device, 1, &m_is_frame_in_flight[m_current_frame_index],
                      VK_TRUE, UINT64_MAX),
      "Fence syncronize failed.");
}

void VKRHI::ResetCommandPool() {
  VK_CHECK(
      vkResetCommandPool(m_device, m_command_pools[m_current_frame_index], 0),
      "Reset command pool failed.");
}

void VKRHI::SubmitRenderingTask() {
  VK_CHECK(vkEndCommandBuffer(m_command_buffers[m_current_frame_index]),
           "Failed to begin command buffer before submit render task.");

  // submit command buffer
  VkPipelineStageFlags wait_stages[] = {
      VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
  VkSubmitInfo submit_info = {};
  submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submit_info.waitSemaphoreCount = 1;
  submit_info.pWaitSemaphores =
      &m_image_render_finish_sema[m_current_frame_index];
  submit_info.pWaitDstStageMask = wait_stages;
  submit_info.commandBufferCount = 1;
  submit_info.pCommandBuffers = &m_command_buffers[m_current_frame_index];
  submit_info.signalSemaphoreCount = 1;
  submit_info.pSignalSemaphores =
      &m_image_present_finish_sema[m_current_frame_index];

  VK_CHECK(
      vkResetFences(m_device, 1, &m_is_frame_in_flight[m_current_frame_index]),
      "Failed to reset fence.");

  VK_CHECK(vkQueueSubmit(m_graphic_queue, 1, &submit_info,
                         m_is_frame_in_flight[m_current_frame_index]),
           "Submit commandbuffer to GPU failed.");

  // present swapchain
  VkPresentInfoKHR present_info = {};
  present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
  present_info.waitSemaphoreCount = 1;
  present_info.pWaitSemaphores =
      &m_image_present_finish_sema[m_current_frame_index];
  present_info.swapchainCount = 1;
  present_info.pSwapchains = &m_swapchain;
  present_info.pImageIndices = &m_current_swapchain_image_index;

  VK_CHECK(vkQueuePresentKHR(m_present_queue, &present_info),
           "Failed to present swapchain image.");

  m_current_frame_index = (m_current_frame_index + 1) % MAX_FRAMES_IN_FLIGHT;
}

void VKRHI::WaitDeviceIdle() { vkDeviceWaitIdle(m_device); }

void VKRHI::CopyBuffer(VkBuffer src, VkDeviceSize src_offset, VkBuffer dst,
                       VkDeviceSize dst_offset, VkDeviceSize size) {
  auto cmd_buf = BeginSingleTimeCommands();
  VkBufferCopy copy_region;
  copy_region.srcOffset = src_offset;
  copy_region.dstOffset = dst_offset;
  copy_region.size = size;
  vkCmdCopyBuffer(cmd_buf, src, dst, 1, &copy_region);
  EndSingleTimeCommands(cmd_buf);
}

void VKRHI::TransitImageLayout(VkImage image, VkImageLayout old_layout,
                               VkImageLayout new_layout, uint32_t layer_count,
                               uint32_t miplevels,
                               VkImageAspectFlags aspect_mask_bits) {
  VkCommandBuffer command_buffer = BeginSingleTimeCommands();

  VkImageMemoryBarrier barrier{VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
  barrier.oldLayout = old_layout;
  barrier.newLayout = new_layout;
  barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.image = image;
  barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  barrier.subresourceRange.baseMipLevel = 0;
  barrier.subresourceRange.levelCount = 1;
  barrier.subresourceRange.baseArrayLayer = 0;
  barrier.subresourceRange.layerCount = layer_count;

  VkPipelineStageFlags sourceStage;
  VkPipelineStageFlags destinationStage;

  if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED &&
      new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

    sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
  } else if (old_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
             new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
  } else {
    throw std::invalid_argument("unsupported layout transition!");
  }

  vkCmdPipelineBarrier(command_buffer, sourceStage, destinationStage, 0, 0,
                       nullptr, 0, nullptr, 1, &barrier);

  EndSingleTimeCommands(command_buffer);
}

VkFormat VKRHI::FindSupportFormat(const std::vector<VkFormat> &candidates,
                                  VkImageTiling tiling,
                                  VkFormatFeatureFlags features) {
  for (VkFormat format : candidates) {
    VkFormatProperties props;
    vkGetPhysicalDeviceFormatProperties(m_physical_device, format, &props);

    if (tiling == VK_IMAGE_TILING_LINEAR &&
        (props.linearTilingFeatures & features) == features) {
      return format;
    } else if (tiling == VK_IMAGE_TILING_OPTIMAL &&
               (props.optimalTilingFeatures & features) == features) {
      return format;
    }
  }

  throw std::runtime_error("findSupportedFormat failed");
}

u_int32_t VKRHI::GetCurrentFrameIndex() const { return m_current_frame_index; };

VkCommandBuffer VKRHI::GetCurrentCommandBuffer() const {
  return m_command_buffers[m_current_frame_index];
}

}  // namespace RHI
}  // namespace ShaderStory
