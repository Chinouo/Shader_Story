#ifndef MATH_HPP
#define MATH_HPP

// #define GLM_FORCE_MESSAGES
#define GLM_FORCE_CXX17
#define GLM_FORCE_PURE
#define GLM_FORCE_SWIZZLE
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_RADIANS
#define GLM_FORCE_SSE2
#include "third_party/glm/glm/glm.hpp"

namespace ShaderStory {

using namespace glm;

constexpr vec3 X_AXIS = {1.f, 0.f, 0.f};
constexpr vec3 Y_AXIS = {0.f, 1.f, 0.f};
constexpr vec3 Z_AXIS = {0.f, 0.f, 1.f};

}  // namespace ShaderStory

#endif