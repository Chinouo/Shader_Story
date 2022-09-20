#ifndef WINDOW_SYSTEM_HPP
#define WINDOW_SYSTEM_HPP

#include <functional>
#include <vector>

#include "engine/common/macros.h"

#ifndef GLFW_INCLUDE_VULKAN
#define GLFW_INCLUDE_VULKAN
#include "third_party/glfw/include/GLFW/glfw3.h"
#endif

namespace ShaderStory {
using KeyCallback = std::function<void(int, int, int, int)>;
using CursorPosCallback = std::function<void(int, int)>;

enum class WindowMode : u_int32_t {
  PLAY,
  EDIT,
};

class WindowSystem final {
 public:
  static void keyCallback(GLFWwindow* window, int key, int scancode, int action,
                          int mods);

  static void cursorPosCallback(GLFWwindow* window, double xpos, double ypos);

  WindowSystem(int width, int height);
  ~WindowSystem();

  void Initialize();
  void Destory();

  // on game play mode
  void InstallCallbacks();
  void UninstallCallbacks();

  // on imgui mode
  void InstallImGuiCallbacks();
  void UninstallImGuiCallbacks();

  /// only used by Vulkan RHI surface create.
  GLFWwindow* GetWindow() const;

  bool ShouldCloseWindow() const;

  // we prefer r-ref, persist callback.
  void registerKeyCallback(KeyCallback&&);
  void registerCurPosCallbakc(CursorPosCallback&&);

  void PerformModeChange(WindowMode target);

 private:
  // internal callback.
  void onKey(int key, int scancode, int action, int mods);
  void onCursorPos(double xpos, double ypos);

  bool isEditMode() const { return mode == WindowMode::EDIT; }

 private:
  GLFWwindow* m_wd;

  // if in edit mode, event is down to imgui, else to our callback.
  WindowMode mode{WindowMode::EDIT};

  int m_width;
  int m_height;

  DISALLOW_IMPLICIT_CONSTRUCTORS(WindowSystem);

  std::vector<KeyCallback> m_key_callbacks;
  std::vector<CursorPosCallback> m_cur_callbacks;
};

}  // namespace ShaderStory

#endif