#version 460 core

#define PI 3.1415926

float D_GGX(const float dotNH, const float roughness) {
  float alpha = roughness * roughness;
  float alpha2 = alpha * alpha;
  float denom = dotNH * dotNH * (alpha2 - 1.0) + 1.0;
  return (alpha2) / (PI * denom * denom);
}

float G_GeometrySmith(const float dotNL, const float dotNV,
                      const float roughness) {
  float r = (roughness + 1.0);
  float k = (r * r) / 8.0;
  float GL = dotNL / (dotNL * (1.0 - k) + k);
  float GV = dotNV / (dotNV * (1.0 - k) + k);
  return GL * GV;
}

float pow5(const float val) { return (val * val) * (val * val) * val; }

vec3 F_Schlick(const float cos_theta, const vec3 F0) {
  return F0 + (1 - F0) * pow5(cos_theta);
}

vec3 PBR_BRDF(const vec3 N, const vec3 V, const vec3 L, const vec3 albedo,
              const float roughness, const float metallic) {
  vec3 Lo = vec3(0.0);
  vec3 F0 = mix(vec3(0.04), albedo, metallic);

  vec3 H = normalize(V + L);
  float dotNV = clamp(dot(N, V), 0.0, 1.0);
  float dotNL = clamp(dot(N, L), 0.0, 1.0);
  float dotLH = clamp(dot(L, H), 0.0, 1.0);
  float dotNH = clamp(dot(N, H), 0.0, 1.0);

  float D = D_GGX(dotNH, roughness);
  vec3 F = F_Schlick(dotNV, F0);
  float G = G_GeometrySmith(dotNL, dotNV, roughness);

  vec3 numerator = D * G * F;
  float denominator = max((4.0 * dotNL * dotNV), 0.001);
  vec3 specular_contribute = numerator / denominator;

  float light_attenuation = 1.0;
  return Lo * light_attenuation;
}