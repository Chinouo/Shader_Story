#ifndef RENDER_PIPELINE_HPP
#define RENDER_PIPELINE_HPP
#include <memory>

#include "engine/common/macros.h"
#include "engine/runtime/render/pass/main_camera_pass.hpp"
#include "engine/runtime/render/pass/sun_pass.hpp"
#include "engine/runtime/render/pass/ui_pass.hpp"
#include "engine/runtime/render/render_base.hpp"
namespace ShaderStory {

class RenderPipeline final {
 public:
  RenderPipeline();
  ~RenderPipeline();

  void Initilaize(std::shared_ptr<RHI::VKRHI> rhi,
                  std::shared_ptr<RenderResource> render_resource);

  void RegisterUIComponent(ReflectUIComponent*);

  void RecreatePipeline();
  void RecordCommands();

 private:
  std::unique_ptr<SunPass> sun_pass;
  std::unique_ptr<UIPass> ui_pass;
  std::unique_ptr<MainCameraPass> main_camera_pass;

  void SetupPasses();
  void DestoryPasses();

  std::shared_ptr<RHI::VKRHI> m_rhi;
  std::shared_ptr<RenderResource> m_resource;

  DISALLOW_COPY_ASSIGN_AND_MOVE(RenderPipeline);
};

};  // namespace ShaderStory

#endif
