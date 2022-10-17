#define PI 3.141592653589793
#define PI2 6.283185307179586

struct PointLight {
  vec3 position;
  float radius;
  vec3 color;
  float intensity;  // power / 4PI = intensity.
};

struct DirectionalLight {
  vec3 direction;
  float _padding_direction;
  vec3 color;
  float _padding_color;
};
