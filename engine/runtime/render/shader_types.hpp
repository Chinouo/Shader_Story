#ifndef SHADER_TYPES_HPP
#define SHADER_TYPES_HPP

#include <vulkan/vulkan.hpp>

#include "engine/core/math.hpp"
namespace ShaderStory {

struct RenderMeshVertexBasic {
  vec3 position;
  vec3 normal;
  vec3 tangent;
  vec2 texcoords;

  static std::array<VkVertexInputBindingDescription, 1>
  GetInputBindingDescription() {
    std::array<VkVertexInputBindingDescription, 1> bindings;
    bindings[0].binding = 0;
    bindings[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    bindings[0].stride = sizeof(RenderMeshVertexBasic);

    return bindings;
  }

  static std::array<VkVertexInputAttributeDescription, 4>
  GetAttributeDescription() {
    std::array<VkVertexInputAttributeDescription, 4> inputs;
    inputs[0].location = 0;
    inputs[0].binding = 0;
    inputs[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    inputs[0].offset = offsetof(RenderMeshVertexBasic, position);

    inputs[1].location = 1;
    inputs[1].binding = 0;
    inputs[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    inputs[1].offset = offsetof(RenderMeshVertexBasic, normal);

    inputs[2].location = 2;
    inputs[2].binding = 0;
    inputs[2].format = VK_FORMAT_R32G32B32_SFLOAT;
    inputs[2].offset = offsetof(RenderMeshVertexBasic, tangent);

    inputs[3].location = 3;
    inputs[3].binding = 0;
    inputs[3].format = VK_FORMAT_R32G32_SFLOAT;
    inputs[3].offset = offsetof(RenderMeshVertexBasic, texcoords);

    return inputs;
  }
};

struct RawMesh {
  std::vector<vec3> positions;
  std::vector<vec3> normals;
  std::vector<vec3> tangents;
  std::vector<vec2> texcoords;

  std::vector<u_int32_t> indices;
};

}  // namespace ShaderStory

#endif