#ifndef RENDER_BASE_HPP
#define RENDER_BASE_HPP
#include <memory>

#include "engine/common/macros.h"
#include "engine/runtime/render/render_resource.hpp"
#include "engine/runtime/render/rhi/vulkan/vk_rhi.hpp"
namespace ShaderStory {

struct RenderPassBackendConfig {
  std::shared_ptr<RHI::VKRHI> rhi;
  std::shared_ptr<RenderResource> render_resource;
};

/// Represent each pass, similar to  VkRenderPass.
class RenderPassBase {
 public:
  /// child need call this to get backend data.
  void PreInitialize(RenderPassBackendConfig backend) {
    m_rhi = backend.rhi;
    m_resources = backend.render_resource;
  }

  virtual void RunPass() = 0;

  virtual ~RenderPassBase(){};

 protected:
  std::shared_ptr<RHI::VKRHI> m_rhi;
  std::shared_ptr<RenderResource> m_resources;
};

};  // namespace ShaderStory

#endif
