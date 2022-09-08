

#ifndef RENDER_SYSTEM_HPP
#define RENDER_SYSTEM_HPP
#include <thread>

#include "engine/common/macros.h"
#include "engine/runtime/render/render_pipeline.hpp"
#include "engine/runtime/render/render_resource.hpp"
#include "engine/runtime/render/rhi/vulkan/vk_rhi.hpp"

namespace ShaderStory {

class RenderSystem final {
 public:
  friend class Engine;

  RenderSystem();
  ~RenderSystem();

  void Initialize();
  void Destory();

  void ReloadPipeline();

  void TickRender(double delta_time);

  void AddPostFrameCallback(std::function<void()>&& closure);

  void ConsumeSwapdata(const SwapData&);

 private:
  // may not needed to use atomic bool
  bool should_shutdown{false};

  std::shared_ptr<RHI::VKRHI> m_rhi;
  std::shared_ptr<RenderResource> m_resources;
  std::unique_ptr<RenderPipeline> m_pipeline;

  std::vector<std::function<void()> > m_post_frame_callbacks;

  DISALLOW_COPY_ASSIGN_AND_MOVE(RenderSystem);
};

}  // namespace ShaderStory

#endif
