#include "engine/runtime/window/window_system.hpp"

namespace ShaderStory {

void WindowSystem::keyCallback(GLFWwindow* window, int key, int scancode,
                               int action, int mods) {
  WindowSystem* wd_sys =
      static_cast<WindowSystem*>(glfwGetWindowUserPointer(window));
  if (wd_sys) wd_sys->onKey(key, scancode, action, mods);
}

void WindowSystem::cursorPosCallback(GLFWwindow* window, double xpos,
                                     double ypos) {
  WindowSystem* wd_sys =
      static_cast<WindowSystem*>(glfwGetWindowUserPointer(window));
  if (wd_sys) wd_sys->onCursorPos(xpos, ypos);
}

WindowSystem::WindowSystem(int width, int height)
    : m_width(width), m_height(height) {}

WindowSystem::~WindowSystem() {}

void WindowSystem::Initialize() {
  ASSERT(glfwInit() && "GLFW init failed.");
  // wd conifg.
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

  m_wd = glfwCreateWindow(m_width, m_height, "ShaderStory", nullptr, nullptr);
  ASSERT(m_wd && "Falied to create GLFW Window.");

  InstallCallbacks();
}

void WindowSystem::Destory() {
  glfwDestroyWindow(m_wd);
  glfwTerminate();
}

bool WindowSystem::ShouldCloseWindow() const {
  return glfwWindowShouldClose(m_wd);
}

GLFWwindow* WindowSystem::GetWindow() const { return m_wd; }

void WindowSystem::InstallCallbacks() {
  glfwSetWindowUserPointer(m_wd, this);
  glfwSetCursorPosCallback(m_wd, WindowSystem::cursorPosCallback);
  glfwSetKeyCallback(m_wd, WindowSystem::keyCallback);
}

void WindowSystem::UninstallCallbacks() {
  glfwSetCursorPosCallback(m_wd, nullptr);
  glfwSetKeyCallback(m_wd, nullptr);
}

void WindowSystem::InstallImGuiCallbacks() {}

void WindowSystem::UninstallImGuiCallbacks() {}

void WindowSystem::registerKeyCallback(KeyCallback&& callback) {
  m_key_callbacks.emplace_back(callback);
}

void WindowSystem::registerCurPosCallbakc(CursorPosCallback&& callback) {
  m_cur_callbacks.emplace_back(callback);
}

void WindowSystem::onKey(int key, int scancode, int action, int mods) {
  for (const auto& callback : m_key_callbacks) {
    callback(key, scancode, action, mods);
  }
}

void WindowSystem::onCursorPos(double xpos, double ypos) {
  for (const auto& callback : m_cur_callbacks) {
    callback(xpos, ypos);
  }
}

}  // namespace ShaderStory