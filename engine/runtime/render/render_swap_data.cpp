#include "engine/runtime/render/render_swap_data.hpp"

namespace ShaderStory {

SwapData& RenderSwapData::GetSwapData(int idx) { return m_swap_data[idx]; }

}  // namespace ShaderStory