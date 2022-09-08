#include "engine/runtime/framework/world_manager.hpp"

#include <iostream>

namespace ShaderStory {

void WorldManager::Initialize() {}

void WorldManager::Dispose() {}

void WorldManager::Tick(double delta_time) {
  m_camera.Tick(delta_time);
  auto pz = m_camera.GetPosition();
  std::cout << pz.x << ' ' << pz.y << ' ' << pz.z << std::endl;
}

void WorldManager::LoadSwapData(SwapData& swap_data) const {
  swap_data.perframe_data.camera_position = m_camera.GetPosition();
  swap_data.perframe_data.proj_view_matrix = m_camera.GetViewProjectionMatrix();
}
}  // namespace ShaderStory