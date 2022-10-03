#include "engine/runtime/io/assets_manager.hpp"

#include <unordered_map>

#include "engine/common/macros.h"
#include "third_party/tinyobjloader/tiny_obj_loader.h"

namespace ShaderStory {

void AssetsManager::LoadObjFile(const std::string& file) {
  using namespace tinyobj;
  ObjReader reader;
  ObjReaderConfig config;
  config.triangulate = true;
  config.vertex_color = false;

  ASSERT(reader.ParseFromFile(file, config));

  auto& attrib = reader.GetAttrib();
  auto& shapes = reader.GetShapes();

  std::vector<StaticMesh> meshes;

  // shapes can be spilt, check Blender split by group.
  for (size_t s = 0; s < shapes.size(); s++) {
    size_t index_offset = 0;

    // remove duplicated data.
    std::unordered_map<MeshVertex, u_int32_t> unique_vertices;
    StaticMesh mesh;
    mesh.name = shapes[s].name;

    for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
      size_t fv = size_t(shapes[s].mesh.num_face_vertices[f]);

      bool with_normal = true;
      bool with_texcoord = true;

      vec3 position[3];
      vec3 normal[3];
      vec2 uv[3];

      assert(fv == 3);
      // triangle only.
      for (size_t v = 0; v < fv; v++) {
        auto idx = shapes[s].mesh.indices[index_offset + v];
        auto vx = attrib.vertices[3 * size_t(idx.vertex_index) + 0];
        auto vy = attrib.vertices[3 * size_t(idx.vertex_index) + 1];
        auto vz = attrib.vertices[3 * size_t(idx.vertex_index) + 2];

        position[v].x = static_cast<float>(vx);
        position[v].y = static_cast<float>(vy);
        position[v].z = static_cast<float>(vz);

        if (idx.normal_index >= 0) {
          auto nx = attrib.normals[3 * size_t(idx.normal_index) + 0];
          auto ny = attrib.normals[3 * size_t(idx.normal_index) + 1];
          auto nz = attrib.normals[3 * size_t(idx.normal_index) + 2];

          normal[v].x = static_cast<float>(nx);
          normal[v].y = static_cast<float>(ny);
          normal[v].z = static_cast<float>(nz);

        } else {
          with_normal = false;
        }

        if (idx.texcoord_index >= 0) {
          auto tx = attrib.texcoords[2 * size_t(idx.texcoord_index) + 0];
          auto ty = attrib.texcoords[2 * size_t(idx.texcoord_index) + 1];

          uv[v].x = static_cast<float>(tx);
          // reverse y for vulkan.
          uv[v].y = static_cast<float>(ty);

        } else {
          with_texcoord = false;
        }
      }
      index_offset += fv;

      // if this mesh not contian normal , we construct one ourself.
      if (!with_normal) {
        vec3 v0 = position[1] - position[0];
        vec3 v1 = position[2] - position[1];

        normal[0] = normalize(cross(v0, v1));
        normal[1] = normal[0];
        normal[2] = normal[0];
      }

      // the same sa nornal.
      if (!with_texcoord) {
        uv[0] = vec2(0.5f, 0.5f);
        uv[1] = uv[0];
        uv[2] = uv[0];
      }

      // we constuct tangent by our selves.
      // see :
      // http://www.opengl-tutorial.org/intermediate-tutorials/tutorial-13-normal-mapping/#:~:text=We%20can%20now%20use%20our%20formula%20to%20compute,Finally%2C%20we%20fill%20the%20%2Atangents%20%2Aand%20%2Abitangents%20%2Abuffers.
      // or :
      // https://gamedev.stackexchange.com/questions/68612/how-to-compute-tangent-and-bitangent-vectors
      vec3 tangent{7, 7, 7};
      {
        vec3 edge1 = position[1] - position[0];
        vec3 edge2 = position[2] - position[0];
        vec2 deltaUV1 = uv[1] - uv[0];
        vec2 deltaUV2 = uv[2] - uv[0];

        auto divide = deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y;
        if (divide >= 0.0f && divide < 0.000001f)
          divide = 0.000001f;
        else if (divide < 0.0f && divide > -0.000001f)
          divide = -0.000001f;

        float df = 1.0f / divide;
        tangent = df * (edge1 * deltaUV2.y - edge2 * deltaUV1.y);
        tangent = normalize(tangent);
      }

      // get 3 vertex data.
      for (size_t i = 0; i < 3; i++) {
        MeshVertex vert{};
        vert.position = position[i];
        vert.normal = normal[i];
        vert.texcoord = uv[i];
        // Gram-Schmidt orthogonalize
        vert.tangent =
            normalize(tangent - (normal[i] * dot(normal[i], tangent)));

        if (!unique_vertices.count(vert)) {
          unique_vertices[vert] = mesh.vertices.size();
          mesh.vertices.push_back(vert);
        }
        mesh.indices.push_back(unique_vertices[vert]);
      }
    }

    meshes.emplace_back(std::move(mesh));
  }
  int x = 0;
}

Texture2D AssetsManager::LoadTextureFile(const std::string& file) {
  Texture2D texture2D{file};
  int channels;
  texture2D.data = stbi_load(file.c_str(), &texture2D.width, &texture2D.height,
                             &channels, STBI_rgb_alpha);
  texture2D.size = texture2D.width * texture2D.height * sizeof(u_int32_t);
  return texture2D;
}

std::vector<StaticMesh> AssetsManager::LoadObjToStaticMeshes(
    const std::string& file) {
  using namespace tinyobj;
  ObjReader reader;
  ObjReaderConfig config;
  config.triangulate = true;
  config.vertex_color = false;

  ASSERT(reader.ParseFromFile(file, config));

  auto& attrib = reader.GetAttrib();
  auto& shapes = reader.GetShapes();

  std::vector<StaticMesh> meshes;

  // shapes can be spilt, check Blender split by group.
  for (size_t s = 0; s < shapes.size(); s++) {
    size_t index_offset = 0;

    // remove duplicated data.
    std::unordered_map<MeshVertex, u_int32_t> unique_vertices;
    StaticMesh mesh;
    mesh.name = shapes[s].name;

    for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
      size_t fv = size_t(shapes[s].mesh.num_face_vertices[f]);

      bool with_normal = true;
      bool with_texcoord = true;

      vec3 position[3];
      vec3 normal[3];
      vec2 uv[3];

      assert(fv == 3);
      // triangle only.
      for (size_t v = 0; v < fv; v++) {
        auto idx = shapes[s].mesh.indices[index_offset + v];
        auto vx = attrib.vertices[3 * size_t(idx.vertex_index) + 0];
        auto vy = attrib.vertices[3 * size_t(idx.vertex_index) + 1];
        auto vz = attrib.vertices[3 * size_t(idx.vertex_index) + 2];

        position[v].x = static_cast<float>(vx);
        position[v].y = static_cast<float>(vy);
        position[v].z = static_cast<float>(vz);

        if (idx.normal_index >= 0) {
          auto nx = attrib.normals[3 * size_t(idx.normal_index) + 0];
          auto ny = attrib.normals[3 * size_t(idx.normal_index) + 1];
          auto nz = attrib.normals[3 * size_t(idx.normal_index) + 2];

          normal[v].x = static_cast<float>(nx);
          normal[v].y = static_cast<float>(ny);
          normal[v].z = static_cast<float>(nz);

        } else {
          with_normal = false;
        }

        if (idx.texcoord_index >= 0) {
          auto tx = attrib.texcoords[2 * size_t(idx.texcoord_index) + 0];
          auto ty = attrib.texcoords[2 * size_t(idx.texcoord_index) + 1];

          uv[v].x = static_cast<float>(tx);
          // reverse y for vulkan.
          uv[v].y = 1.0 - static_cast<float>(ty);

        } else {
          with_texcoord = false;
        }
      }
      index_offset += fv;

      // if this mesh not contian normal , we construct one ourself.
      if (!with_normal) {
        vec3 v0 = position[1] - position[0];
        vec3 v1 = position[2] - position[1];

        normal[0] = normalize(cross(v0, v1));
        normal[1] = normal[0];
        normal[2] = normal[0];
      }

      // the same sa nornal.
      if (!with_texcoord) {
        uv[0] = vec2(0.5f, 0.5f);
        uv[1] = uv[0];
        uv[2] = uv[0];
      }

      // we constuct tangent by our selves.
      // see :
      // http://www.opengl-tutorial.org/intermediate-tutorials/tutorial-13-normal-mapping/#:~:text=We%20can%20now%20use%20our%20formula%20to%20compute,Finally%2C%20we%20fill%20the%20%2Atangents%20%2Aand%20%2Abitangents%20%2Abuffers.
      // or :
      // https://gamedev.stackexchange.com/questions/68612/how-to-compute-tangent-and-bitangent-vectors
      vec3 tangent{7, 7, 7};
      {
        vec3 edge1 = position[1] - position[0];
        vec3 edge2 = position[2] - position[0];
        vec2 deltaUV1 = uv[1] - uv[0];
        vec2 deltaUV2 = uv[2] - uv[0];

        auto divide = deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y;
        if (divide >= 0.0f && divide < 0.000001f)
          divide = 0.000001f;
        else if (divide < 0.0f && divide > -0.000001f)
          divide = -0.000001f;

        float df = 1.0f / divide;
        tangent = df * (edge1 * deltaUV2.y - edge2 * deltaUV1.y);
        tangent = normalize(tangent);
      }

      // get 3 vertex data.
      for (size_t i = 0; i < 3; i++) {
        MeshVertex vert{};
        vert.position = position[i];
        vert.normal = normal[i];
        vert.texcoord = uv[i];
        // Gram-Schmidt orthogonalize
        vert.tangent =
            normalize(tangent - (normal[i] * dot(normal[i], tangent)));

        if (!unique_vertices.count(vert)) {
          unique_vertices[vert] = mesh.vertices.size();
          mesh.vertices.push_back(vert);
        }
        mesh.indices.push_back(unique_vertices[vert]);
      }
    }

    meshes.emplace_back(std::move(mesh));
  }

  return meshes;
}

}  // namespace ShaderStory