#ifndef WINDOW_SYSTEM_HPP
#define WINDOW_SYSTEM_HPP

#include "engine/common/macros.h"

#ifndef GLFW_INCLUDE_VULKAN
#define GLFW_INCLUDE_VULKAN
#include "third_party/glfw/include/GLFW/glfw3.h"
#endif

namespace ShaderStory {

class WindowSystem {
 public:
  WindowSystem(int width, int height);

  ~WindowSystem();

  void Initialize();

  void Destory();

  /// only used by Vulkan RHI surface create.
  GLFWwindow* GetWindow() const;

  bool ShouldCloseWindow() const;

 private:
  GLFWwindow* wd_;

  int m_width;
  int m_height;

  DISALLOW_IMPLICIT_CONSTRUCTORS(WindowSystem);
};

}  // namespace ShaderStory

#endif