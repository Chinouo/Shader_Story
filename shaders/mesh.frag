#version 460

#define LIGHT_WORLD_SIZE 0.005
#define LIGHT_FRUSTUM_WIDTH 3.75
#define LIGHT_SIZE_UV (LIGHT_WORLD_SIZE / LIGHT_FRUSTUM_WIDTH)
#define NEAR_PLANE 9.5

#define NUM_SAMPLES 7
#define NUM_RINGS 11
#define BLOCKER_SEARCH_NUM_SAMPLES NUM_SAMPLES
#define PCF_SAMPLES NUM_SAMPLES

#define EPS 1e-4
#define BIAS 0.0005
#define PI 3.141592653589793
#define PI2 6.283185307179586

layout(set = 0, binding = 0) uniform PerframeData {
  mat4 proj_view_matrix;
  mat4 sun_proj_view_matrix;
  vec3 camera_position_ws;
  vec3 sun_ray_direction;
  vec3 sun_position_ws;
}
perframe_data;

layout(set = 0, binding = 1) uniform sampler2D terrain_texture;

layout(set = 0, binding = 2) uniform sampler2D sun_shadowmap;

layout(location = 0) in vec3 in_frag_position_ws;
layout(location = 1) in vec3 in_frag_normal;
layout(location = 2) in vec2 in_frag_texcoord;
layout(location = 3) in vec4 in_frag_sun_space;
// layout(location = 3) in vec4 in_position_sun_space;
// layout(location = 4) in vec3 in_sun_position_ws;
// layout(location = 5) in vec3 in_sun_ray_dir;

layout(location = 0) out vec4 out_color;

float rand_1to1(float x) {
  // [-1, 1]
  return fract(sin(x) * 10000.0);
}

float rand_2to1(vec2 uv) {
  // [0, 1]
  const float a = 12.9898, b = 78.233, c = 43758.5453;
  float dt = dot(uv.xy, vec2(a, b)), sn = mod(dt, PI);
  return fract(sin(sn) * c);
}

float unpack(vec4 rgbaDepth) {
  const vec4 bitShift = vec4(1.0, 1.0 / 256.0, 1.0 / (256.0 * 256.0),
                             1.0 / (256.0 * 256.0 * 256.0));
  return dot(rgbaDepth, bitShift);
}

vec2 poissonDisk[NUM_SAMPLES];
// pcf sample method 1:
// void p1() {}
// pcf sample method 2:
void initPoissonDiskSamples(const in vec2 randomSeed) {
  float ANGLE_STEP = PI2 * float(NUM_RINGS) / float(NUM_SAMPLES);
  float INV_NUM_SAMPLES = 1.0 / float(NUM_SAMPLES);

  float angle = rand_2to1(randomSeed) * PI2;
  float radius = INV_NUM_SAMPLES;
  float radiusStep = radius;

  for (int i = 0; i < NUM_SAMPLES; i++) {
    poissonDisk[i] = vec2(cos(angle), sin(angle)) * pow(radius, 0.75);
    radius += radiusStep;
    angle += ANGLE_STEP;
  }
}

// make sure dir and normal is normalized.
vec3 Bling_Phong(vec3 light_dir, vec3 view_dir, vec3 normal) {
  // make sure direction for calculate.
  vec3 half_way = normalize(view_dir + light_dir);

  float ambient = 0.2;
  float diffuse = max(dot(normal, light_dir), 0.0);
  float specular = pow(max(dot(normal, half_way), 0.0), 64.0);
  vec3 final_color = (ambient + diffuse + specular) *
                     texture(terrain_texture, in_frag_texcoord).rgb;

  return final_color;
}

// PCF
float PCF_Filter(vec2 uv, float zReceiver, float filterRadius) {
  float sum = 0.0;
  float depth;

  for (int i = 0; i < PCF_SAMPLES; ++i) {
    depth = unpack(texture(sun_shadowmap, uv + poissonDisk[i] * filterRadius));
    if (zReceiver <= depth) sum += 1.0;
  }
  return sum / PCF_SAMPLES;
}

// get average blocker depth.
float findBlocker(const in vec2 uv, const float z_receiver) {
  float searchRadius = LIGHT_SIZE_UV * (z_receiver - NEAR_PLANE) / z_receiver;
  float blockerDepthSum = 0.0;
  int numBlockers = 0;

  for (int i = 0; i < BLOCKER_SEARCH_NUM_SAMPLES; i++) {
    float shadowMapDepth =
        unpack(texture(sun_shadowmap, uv + poissonDisk[i] * searchRadius).rgba);
    if (shadowMapDepth < z_receiver) {
      blockerDepthSum += shadowMapDepth;
      numBlockers++;
    }
  }

  if (numBlockers == 0) return -1.0;

  return blockerDepthSum / float(numBlockers);
}

float penumbraSize(const in float zReceiver,
                   const in float zBlocker) {  // Parallel plane estimation
  return (zReceiver - zBlocker) / zBlocker;
}

// PCSS
float PCSS() {
  // ortho, w is 1.
  vec3 shadowmap_texcoord = in_frag_sun_space.xyz * 0.5 + 0.5;
  float z_receiver = in_frag_sun_space.z - BIAS;
  vec2 uv = shadowmap_texcoord.st;

  // generate sample points.
  initPoissonDiskSamples(uv);

  // step 1: blocker search
  float average_block_depth = findBlocker(uv, z_receiver);

  if (average_block_depth == -1.0) return 1.0;

  float penumbraRatio = penumbraSize(z_receiver, average_block_depth);
  float filterRadius = penumbraRatio * LIGHT_SIZE_UV * NEAR_PLANE / z_receiver;

  return PCF_Filter(uv, z_receiver, filterRadius);
}

void main() {
  vec3 view_dir =
      normalize(perframe_data.camera_position_ws - in_frag_position_ws);

  // BRDF
  vec3 light_dir =
      normalize(perframe_data.sun_position_ws - in_frag_position_ws);

  vec3 brdf = Bling_Phong(light_dir, view_dir, in_frag_normal);
  float visibility = PCSS();
  out_color = vec4(brdf * visibility, 1.0);
}