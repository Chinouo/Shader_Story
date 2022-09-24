#include "engine/component/lights.hpp"

#include <iostream>

#include "engine/runtime/framework/ui_manager.hpp"
namespace ShaderStory {

Sun::Sun() {}

Sun::~Sun() {}

void Sun::SetUpUIComponent() {
  g_runtime_global_context.m_ui_manager->AddUIComponent(this);
}

void Sun::Tick(double delta_time) {
  // if direction is parallel with world up, cross will get Nan.
  m_position = vec3(30.f, 60.1f, 90.f);
  m_direction = normalize(-m_position);
}

// mat4 Sun::GetViewProjMatrix(const RenderCamera& camera) const {
//   float far = camera.GetZfar();
//   float near = camera.GetZnear();
//   float half_fov = camera.GetHalfFov();
//   float tan_half_fov = tan(half_fov);
//   float aspect = camera.GetAspect();

//   float xn = tan_half_fov * near * aspect;
//   float yn = tan_half_fov * near;
//   float xf = tan_half_fov * far * aspect;
//   float yf = tan_half_fov * far;
//   // making bounding box of camera frustum.

//   // camera space frustum point.
//   vec4 near_lt = vec4(-xn, yn, near, 1.0);
//   vec4 near_lb = vec4(-xn, -yn, near, 1.0);
//   vec4 near_rt = vec4(xn, yn, near, 1.0);
//   vec4 near_rb = vec4(xn, -yn, near, 1.0);

//   vec4 far_lt = vec4(-xf, yf, far, 1.0);
//   vec4 far_lb = vec4(-xf, -yf, far, 1.0);
//   vec4 far_rt = vec4(xf, yf, far, 1.0);
//   vec4 far_rb = vec4(xf, -yf, far, 1.0);

//   // covert camera space frustum points to world space.
//   mat4 inversed_camera_view = camera.GetInverseViewMatrix();
//   // lt lb rt rb lt lb rt rb
//   std::array<vec4, 8> camera_frustums_worldspace = {
//       inversed_camera_view * near_lt, inversed_camera_view * near_lb,
//       inversed_camera_view * near_rt, inversed_camera_view * near_rb,
//       inversed_camera_view * far_lt,  inversed_camera_view * far_lb,
//       inversed_camera_view * far_rt,  inversed_camera_view * far_rb,
//   };

//   // rotate to align foward with light direction.
//   mat4 light_view = lookAt(vec3(0.f, 0.f, 0.f), m_direction, world_up);
//   for (size_t i = 0; i < camera_frustums_worldspace.size(); ++i) {
//     camera_frustums_worldspace[i] = light_view *
//     camera_frustums_worldspace[i];
//   }
//   // DebugPrintMatrix4x4(magic_matrix);
//   // DebugPrintMatrix4x4(light_view);

//   // constuct bounding box.
//   float max_x = std::numeric_limits<float>::lowest();
//   ASSERT(max_x < 0);
//   float min_x = std::numeric_limits<float>::max();

//   float max_y = std::numeric_limits<float>::lowest();
//   float min_y = std::numeric_limits<float>::max();

//   float max_z = std::numeric_limits<float>::lowest();
//   float min_z = std::numeric_limits<float>::max();

//   for (size_t i = 0; i < camera_frustums_worldspace.size(); ++i) {
//     max_x = max(max_x, camera_frustums_worldspace[i].x);
//     max_y = max(max_y, camera_frustums_worldspace[i].y);
//     max_z = max(max_z, camera_frustums_worldspace[i].z);

//     min_x = min(min_x, camera_frustums_worldspace[i].x);
//     min_y = min(min_y, camera_frustums_worldspace[i].y);
//     min_z = min(min_z, camera_frustums_worldspace[i].z);
//   }

//   float max_distance = max(
//       length(camera_frustums_worldspace[0] - camera_frustums_worldspace[7]),
//       length(camera_frustums_worldspace[4] - camera_frustums_worldspace[7]));
//   float world_unit_per_pixel = max_distance / 2160.f;
//   float pos_x = (min_x + max_x) * 0.5f;
//   pos_x /= world_unit_per_pixel;
//   pos_x = floor(pos_x);
//   pos_x *= world_unit_per_pixel;

//   float pos_y = (min_y + max_y) * 0.5f;
//   pos_y /= world_unit_per_pixel;
//   pos_y = floor(pos_y);
//   pos_y *= world_unit_per_pixel;

//   float pos_z = min_z;
//   pos_z /= world_unit_per_pixel;
//   pos_z = floor(pos_z);
//   pos_z *= world_unit_per_pixel;

//   vec3 light_pos = vec3(pos_x, pos_y, pos_z);

//   // use camera local space to calculate lookat.
//   auto lkat =
//       translate(lookAt(vec3(0.f, 0.f, 0.f), m_direction, world_up),
//       -light_pos);
//   auto proj = ortho(min_x, max_x, min_y, max_y, 0.f, max_z - min_z);

//   return proj * lkat;
// };

mat4 Sun::GetViewProjMatrixSphereBounding(const RenderCamera& camera) const {
  mat4 inversed_proj_view_matrix =
      camera.GetInverseProjectionViewMatrixCascadeUseOnly();

  std::array<vec3, 8> frustum_corners = ndc_points;
  for (size_t i = 0; i < frustum_corners.size(); ++i) {
    vec4 temp = inversed_proj_view_matrix * vec4(frustum_corners[i], 1.f);
    frustum_corners[i] = temp / temp.w;
  }

  vec3 frustum_center_ws = vec3(0.f, 0.f, 0.f);
  for (size_t i = 0; i < 8; ++i) {
    frustum_center_ws += frustum_corners[i];
  }
  frustum_center_ws /= 8.f;

  float radius = 0.f;

  for (size_t i = 0; i < 8; ++i) {
    float distance = length(frustum_corners[i] - frustum_center_ws);
    radius = max(radius, distance);
  }

  radius = ceil(radius * 16.f) / 16.f;

  glm::vec3 maxExtents = glm::vec3(radius);
  glm::vec3 minExtents = -maxExtents;
  glm::vec3 lightDir = normalize(m_direction);

  // 计算LookAt矩阵
  glm::mat4 lightViewMatrix = glm::lookAt(
      frustum_center_ws + lightDir * minExtents.z, frustum_center_ws, world_up);

  // 计算正交矩阵
  glm::mat4 lightOrthoMatrix;
  lightOrthoMatrix =
      glm::ortho(minExtents.x, maxExtents.x, minExtents.y, maxExtents.y, 0.0f,
                 maxExtents.z - minExtents.z);

  mat4 shadow_view_proj = lightOrthoMatrix * lightViewMatrix;

  // ShadowMap Texel Align
  vec4 shadowOrigin = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
  shadowOrigin = shadow_view_proj * shadowOrigin;
  shadowOrigin = shadowOrigin * (2048.f * 0.5f);
  vec4 roundOrign = glm::round(shadowOrigin);
  vec4 roundOffset = roundOrign - shadowOrigin;
  roundOffset = roundOffset * (2.0f / 2048.f);

  lightOrthoMatrix[3][0] = lightOrthoMatrix[3][0] + roundOffset.x;
  lightOrthoMatrix[3][1] = lightOrthoMatrix[3][1] + roundOffset.y;

  return lightOrthoMatrix * lightViewMatrix;
}

mat4 Sun::GetViewProjMatrixTest(const RenderCamera& camera) const {
  auto m1 = lookAt(vec3(0.f, 0.001f, 100.f), vec3(0.f, 0.f, 0.f), world_up);
  auto m2 = ortho(-150.f, 150.f, -150.f, 150.f, 0.f, 200.f);

  return m2 * m1;
}

std::array<CascadeData, 3> Sun::GetCascadeViewProjMatrices(
    const RenderCamera& camera) const {
  // TODO: hardcode, the cnt of data should follow sun resource object.
  std::array<CascadeData, 3> cascade_data;

  float near_clip = camera.GetZnear();
  float far_clip = camera.GetZfar();
  float clip_range = far_clip - near_clip;
  float min_z = near_clip;
  float max_z = near_clip + clip_range;
  float range = max_z - min_z;
  float ratio = max_z / min_z;
  float cascade_split_lambda = 0.95f;
  float cascade_splits[3];  // count follow shadowmap count
  for (size_t i = 0; i < cascade_data.size(); ++i) {
    float p = (i + 1) / static_cast<float>(cascade_data.size());
    float log = min_z * pow(ratio, p);
    float uniform = min_z + range * p;
    float d = cascade_split_lambda * (log - uniform) + uniform;
    cascade_splits[i] = (d - near_clip) / clip_range;
  }

  float last_split_distance = 0.f;

  for (size_t i = 0; i < cascade_data.size(); ++i) {
    std::array<vec3, 8> frustum_corners = {
        vec3(-1.f, 1.f, 0.f),  vec3(1.f, 1.f, 0.f),   vec3(1.f, -1.f, 0.f),
        vec3(-1.f, -1.f, 0.f), vec3(-1.f, 1.f, 1.f),  vec3(1.f, 1.f, 1.f),
        vec3(1.f, -1.f, 1.f),  vec3(-1.f, -1.f, 1.f),
    };

    mat4 inverse_cam = camera.GetInverseProjectionViewMatrix();

    // convert to world space
    for (size_t i = 0; i < 8; ++i) {
      vec4 temp = inverse_cam * vec4(frustum_corners[i], 1.f);
      frustum_corners[i] = temp / temp.w;
    }

    // split frustums
    float split_dist = cascade_splits[i];
    for (size_t i = 0; i < 4; ++i) {
      vec3 dist = frustum_corners[i + 4] - frustum_corners[i];
      frustum_corners[i + 4] = frustum_corners[i] + dist * split_dist;
      frustum_corners[i] = frustum_corners[i] + dist * last_split_distance;
    }

    // center
    vec3 frustum_center_ws = vec3(0.f, 0.f, 0.f);
    for (size_t i = 0; i < 8; ++i) {
      frustum_center_ws += frustum_corners[i];
    }
    frustum_center_ws /= 8.f;

    // bounding box
    float radius = 0.f;
    for (size_t i = 0; i < 8; ++i) {
      float distance = length(frustum_corners[i] - frustum_center_ws);
      radius = max(radius, distance);
    }

    radius = ceil(radius * 16.f) / 16.f;

    vec3 maxExtents = vec3(radius);
    vec3 minExtents = -maxExtents;
    vec3 lightDir = normalize(m_direction);

    // 计算LookAt矩阵
    glm::mat4 lightViewMatrix =
        glm::lookAt(frustum_center_ws + lightDir * minExtents.z,
                    frustum_center_ws, world_up);

    // 计算正交矩阵
    glm::mat4 lightOrthoMatrix;
    lightOrthoMatrix =
        glm::ortho(minExtents.x, maxExtents.x, minExtents.y, maxExtents.y, 0.0f,
                   maxExtents.z - minExtents.z);
    lightOrthoMatrix[1][1] *= -1.f;

    mat4 shadow_view_proj = lightOrthoMatrix * lightViewMatrix;

    // TODO: find a better solution
    // ShadowMap Texel Align
    vec4 shadowOrigin = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
    shadowOrigin = shadow_view_proj * shadowOrigin;
    shadowOrigin =
        shadowOrigin * (2048.f * 0.5f);  // size follow shdowmap resolution.
    vec4 roundOrign = glm::round(shadowOrigin);
    vec4 roundOffset = roundOrign - shadowOrigin;
    roundOffset = roundOffset * (2.0f / 2048.f);

    lightOrthoMatrix[3][0] = lightOrthoMatrix[3][0] + roundOffset.x;
    lightOrthoMatrix[3][1] = lightOrthoMatrix[3][1] + roundOffset.y;

    cascade_data[i].cascade_proj_view_matrix =
        lightOrthoMatrix * lightViewMatrix;
    cascade_data[i].split_depth = near_clip + split_dist * clip_range;
    last_split_distance = cascade_splits[i];
  }
  return cascade_data;
}

mat4 Sun::MakeCascadeViewProjMatrix(const mat4& view_proj_mat) const {
  mat4 inversed_proj_view_matrix = inverse(view_proj_mat);

  std::array<vec3, 8> frustum_corners = ndc_points;
  for (size_t i = 0; i < frustum_corners.size(); ++i) {
    vec4 temp = inversed_proj_view_matrix * vec4(frustum_corners[i], 1.f);
    frustum_corners[i] = temp / temp.w;
  }

  vec3 frustum_center_ws = vec3(0.f, 0.f, 0.f);
  for (size_t i = 0; i < 8; ++i) {
    frustum_center_ws += frustum_corners[i];
  }
  frustum_center_ws /= 8.f;

  float radius = 0.f;

  for (size_t i = 0; i < 8; ++i) {
    float distance = length(frustum_corners[i] - frustum_center_ws);
    radius = max(radius, distance);
  }

  radius = ceil(radius * 16.f) / 16.f;

  glm::vec3 maxExtents = glm::vec3(radius);
  glm::vec3 minExtents = -maxExtents;
  glm::vec3 lightDir = normalize(m_direction);

  // 计算LookAt矩阵
  glm::mat4 lightViewMatrix = glm::lookAt(
      frustum_center_ws + lightDir * minExtents.z, frustum_center_ws, world_up);

  // 计算正交矩阵
  glm::mat4 lightOrthoMatrix;
  lightOrthoMatrix =
      glm::ortho(minExtents.x, maxExtents.x, minExtents.y, maxExtents.y, 0.0f,
                 maxExtents.z - minExtents.z);

  mat4 shadow_view_proj = lightOrthoMatrix * lightViewMatrix;

  // ShadowMap Texel Align
  vec4 shadowOrigin = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
  shadowOrigin = shadow_view_proj * shadowOrigin;
  shadowOrigin = shadowOrigin * (2048.f * 0.5f);
  vec4 roundOrign = glm::round(shadowOrigin);
  vec4 roundOffset = roundOrign - shadowOrigin;
  roundOffset = roundOffset * (2.0f / 2048.f);

  lightOrthoMatrix[3][0] = lightOrthoMatrix[3][0] + roundOffset.x;
  lightOrthoMatrix[3][1] = lightOrthoMatrix[3][1] + roundOffset.y;

  return lightOrthoMatrix * lightViewMatrix;
}

void Sun::OnDrawUI() const {
  ImGui::Begin("Sun", &display_ui, ImGuiWindowFlags_MenuBar);

  ImGui::Text("Sun Position: x: %.3f y: %.3f z: %.3f", m_position.x,
              m_position.y, m_position.z);

  ImGui::End();
}

}  // namespace ShaderStory
