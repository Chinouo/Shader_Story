#ifndef UI_PASS
#define UI_PASS

#include "engine/common/macros.h"
#include "engine/component/base_component.hpp"
#include "engine/runtime/render/render_base.hpp"

namespace ShaderStory {

class UIPass final : public RenderPassBase {
 public:
  UIPass();
  ~UIPass();

  void Initialize();

  void RunPass() override;

  // special for ui pass.
  void SetVkPass(VkRenderPass);

 private:
  void InitializeImGUIBackend();
  void UploadFonts();

 private:
  VkRenderPass m_pass{VK_NULL_HANDLE};

  DISALLOW_COPY_ASSIGN_AND_MOVE(UIPass);
};

}  // namespace ShaderStory

#endif
