#ifndef SBO_HPP
#define SBO_HPP

#include "engine/core/math.hpp"
#include "engine/runtime/render/render_base.hpp"
namespace ShaderStory {

struct PointLightObject {
  vec4 color;
  vec4 position;
};

// a big chunk storage buffer
struct StorageBufferObject {};

};  // namespace ShaderStory

#endif