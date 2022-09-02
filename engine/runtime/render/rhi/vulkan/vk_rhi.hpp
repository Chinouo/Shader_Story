#ifndef VK_RHI_HPP
#define VK_RHI_HPP

#include <optional>
#include <stdexcept>
#include <vector>
#ifndef GLFW_INCLUDE_VULKAN
#define GLFW_INCLUDE_VULKAN
#include "third_party/glfw/include/GLFW/glfw3.h"
#endif

#include "engine/common/macros.h"
#include "third_party/vma/vk_mem_alloc.h"

namespace ShaderStory {
namespace RHI {

#define MAX_FRAMES_IN_FLIGHT 3

struct QueueFamilyIndices {
  std::optional<u_int32_t> graphic_family;
  std::optional<u_int32_t> present_family;
  bool IsComplete() const {
    return graphic_family.has_value() && present_family.has_value();
  };
};

struct SwapChainSupportDetails {
  VkSurfaceCapabilitiesKHR capabilities{};
  std::vector<VkSurfaceFormatKHR> formats;
  std::vector<VkPresentModeKHR> presentModes;
};

class VKRHI final {
 public:
  friend class RenderPassBase;
  friend class UIPass;

  VKRHI();
  ~VKRHI();

  void Initialize(GLFWwindow* wd);
  void Destory();

  u_int32_t GetCurrentFrameIndex() const;

  VkCommandBuffer GetCurrentCommandBuffer() const;

 private:
  // creation func
  void CreateInstance();
  void CreateDebugMessenger();
  void CreateWindowSurface();
  void InitialzePhysicalDevice();
  void CreateLoginDevice();
  void CreateCommandPool();
  void CreateCommandBuffers();
  void CreateDescriptorPool();
  void CreateSyncPrimitives();

  void CreateSwapchain();
  void CreateSwapchainImageViews();
  void CreateFramebufferImageAndView();
  void CreateAssetAllocator();

  void DestoryDebugMessenger();

  static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
      VkDebugUtilsMessageSeverityFlagBitsEXT, VkDebugUtilsMessageTypeFlagsEXT,
      const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void*);

  static QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice pd,
                                              VkSurfaceKHR surface);

  // Func for swapchain creating.
  SwapChainSupportDetails QuerySwapChainSupport();
  VkExtent2D ChooseSwapchainExtent(const VkSurfaceCapabilitiesKHR&);
  VkPresentModeKHR ChoosePresentMode(const std::vector<VkPresentModeKHR>&);
  VkSurfaceFormatKHR ChooseSurfaceFormat(
      const std::vector<VkSurfaceFormatKHR>&);

  DISALLOW_COPY_ASSIGN_AND_MOVE(VKRHI);

 public:
  // util func
  VkCommandBuffer BeginSingleTimeCommands();
  void EndSingleTimeCommands(VkCommandBuffer);
  void WaitDeviceIdle();

  // render func, call by render_sys in sequences.
  void WaitForFence();
  void ResetCommandPool();
  void WarmUpBeforePass();
  void SubmitRenderingTask();

 public:
  // do not modify member out of this class
  GLFWwindow* wd{nullptr};

  VkInstance m_instance{VK_NULL_HANDLE};

  VkPhysicalDevice m_physical_device{VK_NULL_HANDLE};
  VkPhysicalDeviceProperties m_pd_property{};
  VkPhysicalDeviceFeatures m_pd_features{};
  VkPhysicalDeviceMemoryProperties m_pd_memory_property{};

  QueueFamilyIndices m_queue_indices;
  VkDevice m_device{VK_NULL_HANDLE};
  VkQueue m_graphic_queue{VK_NULL_HANDLE};
  VkQueue m_present_queue{VK_NULL_HANDLE};

  VkSurfaceKHR m_surface{VK_NULL_HANDLE};

  VkDescriptorPool m_descriptor_pool{VK_NULL_HANDLE};
  VkCommandPool m_persist_command_pool{VK_NULL_HANDLE};
  VkCommandPool m_command_pools[MAX_FRAMES_IN_FLIGHT];
  VkCommandBuffer m_command_buffers[MAX_FRAMES_IN_FLIGHT];

  // sync  member
  VkSemaphore m_image_render_finish_sema[MAX_FRAMES_IN_FLIGHT];
  VkSemaphore m_image_present_finish_sema[MAX_FRAMES_IN_FLIGHT];
  VkFence m_is_frame_in_flight[MAX_FRAMES_IN_FLIGHT];

  u_int32_t m_current_swapchain_image_index;
  u_int32_t m_current_frame_index{0};

  // swapchain member
  SwapChainSupportDetails m_swapchain_details;
  VkSwapchainKHR m_swapchain{VK_NULL_HANDLE};
  VkFormat m_swapchain_format{VK_FORMAT_UNDEFINED};
  VkExtent2D m_swapchain_extent;
  std::vector<VkImage> m_swapchain_images;
  std::vector<VkImageView> m_swapchain_imageviews;
  VkRect2D m_scissor;

  VmaAllocator m_vma_allocator{VK_NULL_HANDLE};

  VkDebugUtilsMessengerEXT m_debug_messenger{VK_NULL_HANDLE};
};

}  // namespace RHI

}  // namespace ShaderStory

#endif