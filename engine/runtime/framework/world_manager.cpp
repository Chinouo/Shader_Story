#include "engine/runtime/framework/world_manager.hpp"

#include <iostream>

#include "engine/runtime/framework/ui_manager.hpp"

namespace ShaderStory {

void WorldManager::Initialize() {
  m_illumination_manager = std::make_unique<Illumination>();
  sun.SetUpUIComponent();
  m_camera.SetUpRenderRenderCameraUI();
}

void WorldManager::Dispose() {}

void WorldManager::Tick(double delta_time) {
  m_camera.Tick(delta_time);
  auto pz = m_camera.GetPosition();
  sun.Tick(delta_time);
  // std::cout << pz.x << ' ' << pz.y << ' ' << pz.z << std::endl;
}

void WorldManager::LoadSwapData(SwapData& swap_data) const {
  auto cascade_data = sun.GetCascadeViewProjMatrices(m_camera);
  for (int i = 0; i < cascade_data.size(); ++i) {
    swap_data.perframe_ubo_data.cascade_proj_view_mats[i] =
        cascade_data[i].cascade_proj_view_matrix;
    swap_data.perframe_ubo_data.depth_splits[i] = cascade_data[i].split_depth;
  }

  swap_data.perframe_ubo_data.proj_mat = m_camera.GetProjMatrix();
  swap_data.perframe_ubo_data.view_mat = m_camera.GetViewMatrix();

  swap_data.perframe_ubo_data.sun_ray_dir = sun.GetDirection();
  swap_data.perframe_ubo_data.sun_pos_ws = sun.GetPosition();
  swap_data.perframe_ubo_data.camera_pos_ws = m_camera.GetPosition();

  swap_data.perframe_ubo_data.proj_view_mat =
      m_camera.GetViewProjectionMatrix();

  // load light data to sbo.
  m_illumination_manager->LoadSwapData(swap_data.perframe_sbo_data);

  ;
}
}  // namespace ShaderStory
