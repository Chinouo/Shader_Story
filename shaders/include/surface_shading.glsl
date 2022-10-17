float saturate(float val) { return clamp(val, 0.0, 1.0); }

float pow5(float x) {
  float x2 = x * x;
  return x2 * x2 * x;
}

/// shadow

/// BRDF
float D_GGX(float NoH, float roughness) {
  float alpha = roughness * roughness;
  float alpha2 = alpha * alpha;
  float denom = NoH * NoH * (alpha2 - 1.0) + 1.0;
  return (alpha2) / (PI * denom * denom);
}

float G_SchlickSmithGGX(float NoL, float NoV, float roughness) {
  float r = (roughness + 1.0);
  float k = (r * r) / 8.0;
  float GL = NoL / (NoL * (1.0 - k) + k);
  float GV = NoV / (NoV * (1.0 - k) + k);
  return GL * GV;
}

vec3 F_Schlick(float cos_theta, vec3 F0) {
  return F0 + (1.0 - F0) * pow5(1.0 - cos_theta);
}

float distribution(float NoL, float NoV, float NoH, float roughness) {
  return D_GGX(NoH, roughness);
}

float gemetryshadowmask(float NoL, float NoV, float roughness) {
  return G_SchlickSmithGGX(NoL, NoV, roughness);
}

vec3 fresnel(vec3 F0, float NoV) { return F_Schlick(NoV, F0); }

vec3 specularLobe(float NoV, float NoL, float NoH, float roughness,
                  float metallic, vec3 base_color) {
  float D = distribution(NoL, NoV, NoH, roughness);

  float G = gemetryshadowmask(NoL, NoV, roughness);

  // TODO: replace 0.04 with specular texture?
  vec3 F0 = mix(vec3(0.04), base_color, metallic);
  vec3 F = fresnel(F0, NoV);

  vec3 specular_contribute = D * F * G;
  float denom = 4 * NoL * NoV + 0.000001;

  return specular_contribute / denom;
}

float Fd_Lambert() { return 1.0 / PI; }

vec3 diffuseLobe(vec3 base_color) { return base_color * Fd_Lambert(); }

/// END BRDF

/// cook-torance microfacet.
/// all input should be normalized.
vec3 surface_shading(vec3 V, vec3 L, vec3 N, vec3 base_color, float roughness,
                     float metallic) {
  vec3 H = normalize(V + L);
  float NoV = saturate(dot(N, V));
  float NoL = saturate(dot(N, L));
  float NoH = saturate(dot(N, H));

  vec3 Fd = diffuseLobe(base_color);

  if (NoV <= 0.0) return vec3(0.0, 0.0, 0.0);

  // TODO: fix high roughness engine lose in Fr.
  // Bug: Fr look wired?
  vec3 Fr = specularLobe(NoV, NoL, NoH, roughness, metallic, base_color);

  vec3 result_color = Fr + Fd;

  return result_color;
}