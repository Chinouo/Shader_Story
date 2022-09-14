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
#include <array>

#include "third_party/glm/glm/ext.hpp"
#include "third_party/glm/glm/glm.hpp"
#include "third_party/glm/glm/gtx/hash.hpp"

namespace ShaderStory {

using namespace glm;

constexpr vec3 X_AXIS = {1.f, 0.f, 0.f};
constexpr vec3 Y_AXIS = {0.f, 1.f, 0.f};
constexpr vec3 Z_AXIS = {0.f, 0.f, 1.f};

constexpr vec3 world_up = Z_AXIS;
constexpr vec3 world_front = Y_AXIS;
constexpr vec3 world_right = X_AXIS;

// NDC Cube.
constexpr vec3 ndc_far_lt = {-1.f, 1.f, 1.f};
constexpr vec3 ndc_far_lb = {-1.f, -1.f, 1.f};
constexpr vec3 ndc_far_rt = {1.f, 1.f, 1.f};
constexpr vec3 ndc_far_rb = {1.f, -1.f, 1.f};

constexpr vec3 ndc_near_lt = {-1.f, 1.f, 0.f};
constexpr vec3 ndc_near_lb = {-1.f, -1.f, 0.f};
constexpr vec3 ndc_near_rt = {1.f, 1.f, 0.f};
constexpr vec3 ndc_near_rb = {1.f, -1.f, 0.f};

constexpr std::array<vec3, 8> ndc_points = {
    ndc_far_lt,  ndc_far_lb,  ndc_far_rt,  ndc_far_rb,
    ndc_near_lt, ndc_near_lb, ndc_near_rt, ndc_near_rb,
};

// for debug gpu data.
constexpr mat4 magic_matrix = {
    {6.f, 6.f, 6.f, 6.f},
    {7.f, 7.f, 7.f, 7.f},
    {8.f, 8.f, 8.f, 8.f},
    {9.f, 9.f, 9.f, 9.f},

};

inline void DebugPrintVector4(const vec4& vec) {
  printf("%.5f | %.5f | %.5f | %.5f \n", vec.x, vec.y, vec.z, vec.w);
}

inline void DebugPrintMatrix4x4(const mat4& mat) {
  for (size_t i = 0; i < mat.length(); ++i) {
    DebugPrintVector4(mat[i]);
  }
}

}  // namespace ShaderStory

#endif