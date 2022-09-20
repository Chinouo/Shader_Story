#include "engine/component/base_component.hpp"

#include "engine/runtime/io/input_system.hpp"

namespace ShaderStory {
GameCommandReceiver::GameCommandReceiver() {
  g_runtime_global_context.m_input_sys->AddGameCommandReceiver(this);
};

GameCommandReceiver::~GameCommandReceiver() {
  g_runtime_global_context.m_input_sys->RemoverGameCommandReceiver(this);
};

void GameCommandReceiver::onReceiveGameCommand(u_int32_t command) {
  ASSERT(false);
}

}  // namespace ShaderStory
