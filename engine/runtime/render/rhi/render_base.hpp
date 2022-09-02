#ifndef RENDER_BASE_HPP
#define RENDER_BASE_HPP
#include <memory>

#include "engine/common/macros.h"
#include "engine/runtime/render/rhi/vulkan/vk_rhi.hpp"
namespace ShaderStory {

struct RenderPassBackend {
  std::shared_ptr<RHI::VKRHI> rhi;
};

/// Represent each pass, similar to  VkRenderPass.
class RenderPassBase {
 public:
  /// child may call this to get rhi
  void PreInitialize(std::shared_ptr<RHI::VKRHI>& rhi) { m_rhi = rhi; }
  // virtual void SetPassResource() = 0;
  virtual void RunPass() = 0;
  virtual void Dispose() = 0;

  virtual ~RenderPassBase(){};

 protected:
  std::shared_ptr<RHI::VKRHI> m_rhi;
};

/// Represent a entirely render result.
/// may contains multipass.
class RenderPipelineBase {
 public:
  virtual void Initilaize() = 0;
  virtual void Dispose() = 0;
};

};  // namespace ShaderStory

#endif