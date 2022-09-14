#include "engine/component/lights.hpp"

#include <iostream>
namespace ShaderStory {
void Sun::Tick(double delta_time) {
  // if direction is parallel with world up, cross will get Nan.
  m_position = vec3(30.f, 60.1f, 90.f);
  m_direction = normalize(-m_position);
}

mat4 Sun::GetViewProjMatrix(const RenderCamera& camera) const {
  float far = camera.GetZfar();
  float near = camera.GetZnear();
  float half_fov = camera.GetHalfFov();
  float tan_half_fov = tan(half_fov);
  float aspect = camera.GetAspect();

  float xn = tan_half_fov * near * aspect;
  float yn = tan_half_fov * near;
  float xf = tan_half_fov * far * aspect;
  float yf = tan_half_fov * far;
  // making bounding box of camera frustum.

  // camera space frustum point.
  vec4 near_lt = vec4(-xn, yn, near, 1.0);
  vec4 near_lb = vec4(-xn, -yn, near, 1.0);
  vec4 near_rt = vec4(xn, yn, near, 1.0);
  vec4 near_rb = vec4(xn, -yn, near, 1.0);

  vec4 far_lt = vec4(-xf, yf, far, 1.0);
  vec4 far_lb = vec4(-xf, -yf, far, 1.0);
  vec4 far_rt = vec4(xf, yf, far, 1.0);
  vec4 far_rb = vec4(xf, -yf, far, 1.0);

  // covert camera space frustum points to world space.
  mat4 inversed_camera_view = camera.GetInverseViewMatrix();
  // lt lb rt rb lt lb rt rb
  std::array<vec4, 8> camera_frustums_worldspace = {
      inversed_camera_view * near_lt, inversed_camera_view * near_lb,
      inversed_camera_view * near_rt, inversed_camera_view * near_rb,
      inversed_camera_view * far_lt,  inversed_camera_view * far_lb,
      inversed_camera_view * far_rt,  inversed_camera_view * far_rb,
  };

  // rotate to align foward with light direction.
  mat4 light_view = lookAt(vec3(0.f, 0.f, 0.f), m_direction, world_up);
  for (size_t i = 0; i < camera_frustums_worldspace.size(); ++i) {
    camera_frustums_worldspace[i] = light_view * camera_frustums_worldspace[i];
  }
  // DebugPrintMatrix4x4(magic_matrix);
  // DebugPrintMatrix4x4(light_view);

  // constuct bounding box.
  float max_x = std::numeric_limits<float>::lowest();
  ASSERT(max_x < 0);
  float min_x = std::numeric_limits<float>::max();

  float max_y = std::numeric_limits<float>::lowest();
  float min_y = std::numeric_limits<float>::max();

  float max_z = std::numeric_limits<float>::lowest();
  float min_z = std::numeric_limits<float>::max();

  for (size_t i = 0; i < camera_frustums_worldspace.size(); ++i) {
    max_x = max(max_x, camera_frustums_worldspace[i].x);
    max_y = max(max_y, camera_frustums_worldspace[i].y);
    max_z = max(max_z, camera_frustums_worldspace[i].z);

    min_x = min(min_x, camera_frustums_worldspace[i].x);
    min_y = min(min_y, camera_frustums_worldspace[i].y);
    min_z = min(min_z, camera_frustums_worldspace[i].z);
  }

  float max_distance = max(
      length(camera_frustums_worldspace[0] - camera_frustums_worldspace[7]),
      length(camera_frustums_worldspace[4] - camera_frustums_worldspace[7]));
  float world_unit_per_pixel = max_distance / 2160.f;
  float pos_x = (min_x + max_x) * 0.5f;
  pos_x /= world_unit_per_pixel;
  pos_x = floor(pos_x);
  pos_x *= world_unit_per_pixel;

  float pos_y = (min_y + max_y) * 0.5f;
  pos_y /= world_unit_per_pixel;
  pos_y = floor(pos_y);
  pos_y *= world_unit_per_pixel;

  float pos_z = min_z;
  pos_z /= world_unit_per_pixel;
  pos_z = floor(pos_z);
  pos_z *= world_unit_per_pixel;

  vec3 light_pos = vec3(pos_x, pos_y, pos_z);

  // use camera local space to calculate lookat.
  auto lkat =
      translate(lookAt(vec3(0.f, 0.f, 0.f), m_direction, world_up), -light_pos);
  auto proj = ortho(min_x, max_x, min_y, max_y, 0.f, max_z - min_z);

  return proj * lkat;
};

mat4 Sun::GetViewProjMatrixSphereBounding(const RenderCamera& camera) const {
  mat4 inversed_proj_view_matrix = camera.GetInversedProjectionMatrix();

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

}  // namespace ShaderStory
