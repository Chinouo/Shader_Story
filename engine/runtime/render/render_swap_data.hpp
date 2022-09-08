#ifndef RENDER_SWAP_DATA_HPP
#define RENDER_SWAP_DATA_HPP

#include <array>

#include "engine/core/math.hpp"
namespace ShaderStory {

typedef struct {
  mat4 proj_view_matrix;
  vec3 camera_position;
} PerframeData;

struct SwapData {
  PerframeData perframe_data;
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