#include "engine/runtime/window/window_system.hpp"

namespace ShaderStory {

WindowSystem::WindowSystem(int width, int height)
    : m_width(width), m_height(height) {}

WindowSystem::~WindowSystem() {}

void WindowSystem::Initialize() {
  ASSERT(glfwInit() && "GLFW init failed.");
  // wd conifg.
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

  wd_ = glfwCreateWindow(m_width, m_height, "ShaderStory", nullptr, nullptr);
  ASSERT(wd_ && "Falied to create GLFW Window.");
}

void WindowSystem::Destory() {
  glfwDestroyWindow(wd_);
  glfwTerminate();
}

bool WindowSystem::ShouldCloseWindow() const {
  return glfwWindowShouldClose(wd_);
}

GLFWwindow* WindowSystem::GetWindow() const { return wd_; }

}  // namespace ShaderStory