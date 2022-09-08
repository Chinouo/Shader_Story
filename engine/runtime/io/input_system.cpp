#include "engine/runtime/io/input_system.hpp"

#include <iostream>

#include "engine/component/base_component.hpp"
#include "engine/runtime/global/global.hpp"
namespace ShaderStory {

void InputSystem::Initialize() {
  using namespace std::placeholders;
  g_runtime_global_context.m_window_sys->registerKeyCallback(
      std::bind(&InputSystem::onKeyEvent, this, _1, _2, _3, _4));
  g_runtime_global_context.m_window_sys->registerCurPosCallbakc(
      std::bind(&InputSystem::onCursorPos, this, _1, _2));
}

void InputSystem::Dispose() {}

void InputSystem::onKeyEvent(int key, int scancode, int action, int mods) {
  if (action == GLFW_PRESS) {
    switch (key) {
      case GLFW_KEY_W: {
        m_command_state |= (u_int32_t)GameCommand::forward;
      } break;
      case GLFW_KEY_A: {
        m_command_state |= (u_int32_t)GameCommand::left;
      } break;
      case GLFW_KEY_S: {
        m_command_state |= (u_int32_t)GameCommand::backward;
      } break;
      case GLFW_KEY_D: {
        m_command_state |= (u_int32_t)GameCommand::right;
      } break;
    }
  } else if (action == GLFW_RELEASE) {
    switch (key) {
      case GLFW_KEY_W: {
        m_command_state &=
            k_complement_control_command ^ (u_int32_t)GameCommand::forward;
      } break;
      case GLFW_KEY_A: {
        m_command_state &=
            k_complement_control_command ^ (u_int32_t)GameCommand::left;
      } break;
      case GLFW_KEY_S: {
        m_command_state &=
            k_complement_control_command ^ (u_int32_t)GameCommand::backward;
      } break;
      case GLFW_KEY_D: {
        m_command_state &=
            k_complement_control_command ^ (u_int32_t)GameCommand::right;
      } break;
    }
  }
}

void InputSystem::AddGameCommandReceiver(GameCommandReceiver* receiver) {
  m_receivers.push_back(receiver);
}

void InputSystem::RemoverGameCommandReceiver(GameCommandReceiver* receiver) {
  ASSERT(false);
}

void InputSystem::onCursorPos(double x, double y) {
  m_cursor_position_state.x = x;
  m_cursor_position_state.y = y;
}

// void InputSystem::Tick() {}

}  // namespace ShaderStory