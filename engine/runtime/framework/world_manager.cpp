#include "engine/runtime/framework/world_manager.hpp"

#include <iostream>

namespace ShaderStory {

void WorldManager::Initialize() {}

void WorldManager::Dispose() {}

void WorldManager::Tick(double delta_time) {
  m_camera.Tick(delta_time);
  auto pz = m_camera.GetPosition();
  sun.Tick(delta_time);
  // std::cout << pz.x << ' ' << pz.y << ' ' << pz.z << std::endl;
}

void WorldManager::LoadSwapData(SwapData& swap_data) const {
  swap_data.perframe_data.sun_ray_direction = sun.GetDirection();
  swap_data.perframe_data.sun_position_ws = sun.GetPosition();
  swap_data.perframe_data.camera_position_ws = m_camera.GetPosition();
  // swap_data.perframe_data.proj_view_matrix =
  //     m_camera.DebugGetViewProjectionMatrix();
  swap_data.perframe_data.proj_view_matrix = m_camera.GetViewProjectionMatrix();
  // swap_data.perframe_data.sun_proj_view_matrix =
  //     sun.GetViewProjMatrix(m_camera);
  swap_data.perframe_data.sun_proj_view_matrix =
      sun.GetViewProjMatrixSphereBounding(m_camera);
  // swap_data.perframe_data.sun_proj_view_matrix =
  //     sun.GetViewProjMatrixTest(m_camera);
}
}  // namespace ShaderStory