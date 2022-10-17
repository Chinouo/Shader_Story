#include "engine/runtime/window/window_system.hpp"

#include "third_party/glfw/include/GLFW/glfw3.h"
#include "third_party/imgui/backends/imgui_impl_glfw.h"
#include "third_party/imgui/imgui.h"

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

WindowSystem::~WindowSystem() {
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();
}

void WindowSystem::Initialize() {
  ASSERT(glfwInit() && "GLFW init failed.");
  // wd conifg.
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

  m_wd = glfwCreateWindow(m_width, m_height, "ShaderStory", nullptr, nullptr);
  ASSERT(m_wd && "Falied to create GLFW Window.");

  glfwSetWindowUserPointer(m_wd, this);

  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO();
  (void)io;
  ImGui::StyleColorsDark();
  ImGui_ImplGlfw_InitForVulkan(m_wd, true);
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
  glfwSetInputMode(m_wd, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  glfwSetWindowUserPointer(m_wd, this);
  glfwSetCursorPosCallback(m_wd, WindowSystem::cursorPosCallback);
  glfwSetKeyCallback(m_wd, WindowSystem::keyCallback);
}

void WindowSystem::UninstallCallbacks() {
  glfwSetWindowUserPointer(m_wd, nullptr);
  glfwSetCursorPosCallback(m_wd, nullptr);
  glfwSetKeyCallback(m_wd, nullptr);
}

void WindowSystem::InstallImGuiCallbacks() {
  glfwSetInputMode(m_wd, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
  ImGui_ImplGlfw_InstallCallbacks(m_wd);
  ImGuiIO& io = ImGui::GetIO();
  io.ConfigFlags ^= ImGuiConfigFlags_NoMouse;
}

void WindowSystem::UninstallImGuiCallbacks() {
  ImGui_ImplGlfw_RestoreCallbacks(m_wd);
  ImGuiIO& io = ImGui::GetIO();
  io.ConfigFlags |= ImGuiConfigFlags_NoMouse;
}

void WindowSystem::PerformModeChange(WindowMode target) {
  if (target == WindowMode::PLAY) {
    UninstallImGuiCallbacks();
    InstallCallbacks();
    mode = WindowMode::PLAY;

  } else if (target == WindowMode::EDIT) {
    UninstallCallbacks();
    InstallImGuiCallbacks();
    mode = WindowMode::EDIT;
  } else {
    assert(false);
  }
}

void WindowSystem::registerKeyCallback(KeyCallback&& callback) {
  m_key_callbacks.emplace_back(callback);
}

void WindowSystem::registerCurPosCallbakc(CursorPosCallback&& callback) {
  m_cur_callbacks.emplace_back(callback);
}

void WindowSystem::onKey(int key, int scancode, int action, int mods) {
  if (isEditMode()) return;
  if (key == GLFW_KEY_ESCAPE) {
    PerformModeChange(WindowMode::EDIT);
    return;
  }

  for (const auto& callback : m_key_callbacks) {
    callback(key, scancode, action, mods);
  }
}

void WindowSystem::onCursorPos(double xpos, double ypos) {
  if (isEditMode()) return;
  for (const auto& callback : m_cur_callbacks) {
    callback(xpos, ypos);
  }
}

}  // namespace ShaderStory
