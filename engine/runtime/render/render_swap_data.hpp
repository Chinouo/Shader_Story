#ifndef RENDER_SWAP_DATA_HPP
#define RENDER_SWAP_DATA_HPP

#include <array>

#include "engine/component/lights.hpp"
#include "engine/core/math.hpp"

#define CASCADE_COUNT 3

#define MAX_POINT_LIGHT_SIZE 5
#define MAX_DIRECTION_LIGHT_SIZE 5

namespace ShaderStory {

struct PerframeDataUBO {
  // camera
  mat4 proj_view_mat;
  mat4 proj_mat;
  mat4 view_mat;
  // shadow(sun)
  mat4 cascade_proj_view_mats[CASCADE_COUNT];
  vec4 depth_splits;  // float[3], use vec.xyz
  vec3 camera_pos_ws;
  float padding_1;
  vec3 sun_ray_dir;
  float padding_2;
  vec3 sun_pos_ws;
  float padding_3;
};

// struct PointLightObject {
//   vec3 position;
//   float radius;
//   vec3 color;
//   float intensity;
// };

struct DirectionObject {
  vec3 position;
  float padding_1;
  vec3 color;
  float padding_2;
};

struct PerframeDataSBO {
  PointLightSwapData point_lights[MAX_POINT_LIGHT_SIZE];
  DirectionObject direction_lights[MAX_DIRECTION_LIGHT_SIZE];
  uint32_t current_point_light_count;
  uint32_t current_direction_light_count;
};

struct SwapData {
  PerframeDataUBO perframe_ubo_data;
  PerframeDataSBO perframe_sbo_data;
};

/// @brief  Sharing data between Logic thread and Render thread.
/// swap data size is depend on your semphere, as well as is logic can run ahead
/// of render how many frames in logic, this data only used for rendering.
class RenderSwapData final {
 public:
  SwapData& GetSwapData(int logic_frame_index);

  std::array<SwapData, 2> m_swap_data;
};

};  // namespace ShaderStory

#endif
