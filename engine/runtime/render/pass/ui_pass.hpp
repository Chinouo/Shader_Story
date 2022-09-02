#ifndef UI_PASS
#define UI_PASS

#include "engine/common/macros.h"
#include "engine/runtime/render/rhi/render_base.hpp"

namespace ShaderStory {

class UIPass final : public RenderPassBase {
 public:
  UIPass() = default;
  ~UIPass() = default;

  void Initialize();

  void RunPass() override;

  // special for ui pass.
  void SetVkPass(VkRenderPass);

  void Dispose() override;

 private:
  void InitializeImGUIBackend();
  void UploadFonts();

  VkRenderPass m_pass{VK_NULL_HANDLE};

  DISALLOW_COPY_ASSIGN_AND_MOVE(UIPass);
};

}  // namespace ShaderStory

#endif