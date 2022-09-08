#ifndef ASSETS_MANAGER_HPP
#define ASSETS_MANAGER_HPP
#include <functional>
#include <string>
#include <vector>
#include <vulkan/vulkan.hpp>

#include "engine/common/macros.h"
#include "engine/core/math.hpp"
#include "third_party/stb/stb_image.h"

namespace ShaderStory {

/// @brief  only used for asset load.
struct MeshVertex {
  vec3 position;
  vec3 normal;
  vec3 tangent;
  vec2 texcoord;

  // ignore tangent
  bool operator==(const MeshVertex& rhs) const {
    return position == rhs.position && normal == rhs.normal &&
           texcoord == rhs.texcoord;
  }

  static std::size_t GetHash(const MeshVertex& v) {
    using namespace std;
    return (hash<vec3>()(v.position) ^ (hash<vec3>()(v.normal) << 1) >> 1) ^
           (hash<vec2>()(v.texcoord) << 1);
  }
};

/// @brief Loading static mesh using tinyobj.
struct StaticMesh {
  std::string name;
  std::vector<MeshVertex> vertices;
  std::vector<u_int32_t> indices;

  u_int32_t GetVerticesSize() const {
    return vertices.size() * sizeof(MeshVertex);
  }
  u_int32_t GetIndicesSize() const {
    return indices.size() * sizeof(u_int32_t);
  }
};

/// Texture2D  RGBA
/// TODO: use move constrcutor.
struct Texture2D {
  std::string name;
  int width;
  int height;
  void* data{nullptr};
  u_int32_t size{0};

  void Dispose() { stbi_image_free(data); }

  VkImageCreateInfo GetDefaultImageCreateInfo() {
    VkImageCreateInfo info{VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
    info.imageType = VK_IMAGE_TYPE_2D;
    info.format = VK_FORMAT_R8G8B8A8_SRGB;
    info.extent.depth = 1;
    info.extent.width = width;
    info.extent.height = height;
    info.mipLevels = 1;
    info.arrayLayers = 1;
    info.samples = VK_SAMPLE_COUNT_1_BIT;
    info.tiling = VK_IMAGE_TILING_OPTIMAL;
    info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    info.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
    return info;
  }
};

class AssetsManager final {
 public:
  static void LoadObjFile(const std::string&);
  static void LoadTextureFile();
  static std::vector<StaticMesh> LoadObjToStaticMeshes(const std::string&);
  static Texture2D LoadTextureFile(const std::string&);

 private:
};

}  // namespace ShaderStory

template <>
struct std::hash<ShaderStory::MeshVertex> {
  std::size_t operator()(ShaderStory::MeshVertex const& v) const noexcept {
    return ShaderStory::MeshVertex::GetHash(v);
  }
};

#endif
