#ifndef RENDER_SWAP_DATA_HPP
#define RENDER_SWAP_DATA_HPP

#include <array>

#include "engine/core/math.hpp"

#define CASCADE_COUNT 3

namespace ShaderStory {

typedef struct {
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
} PerframeDataUBO;

// /// @brief  Double check your maxUniformBufferRange
// typedef struct {
//   mat4 proj_view_matrix;
//   mat4 camera_view_matrix;
//   mat4 cascade_proj_view_matrices[CASCADE_COUNT];
//   alignas(16) float depth_splits[CASCADE_COUNT];
//   alignas(16) vec3 camera_position_ws;
//   alignas(16) vec3 sun_ray_direction;
//   alignas(16) vec3 sun_position_ws;

// } PerframeData;

// for big chunk dynamic data.
typedef struct {
} PerframeStorageBufferData;

struct SwapData {
  PerframeDataUBO perframe_data;
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
