#ifndef WORLD_MANAGER_HPP
#define WORLD_MANAGER_HPP

#include "engine/component/camera.hpp"
#include "engine/runtime/render/render_swap_data.hpp"

namespace ShaderStory {

/// @brief  manager all game entity and component.
class WorldManager final {
 public:
  WorldManager() = default;
  ~WorldManager() = default;

  void Initialize();
  void Dispose();

  void Tick(double delta_time);

  void LoadSwapData(SwapData&) const;

 private:
  CameraComponent m_camera;

  DISALLOW_COPY_ASSIGN_AND_MOVE(WorldManager);
};

}  // namespace ShaderStory

#endif