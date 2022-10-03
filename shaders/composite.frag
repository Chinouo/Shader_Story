#version 460

#define LIGHT_WORLD_SIZE 0.005
#define LIGHT_FRUSTUM_WIDTH 3.75
#define LIGHT_SIZE_UV (LIGHT_WORLD_SIZE / LIGHT_FRUSTUM_WIDTH)
#define NEAR_PLANE 9.5

#define SAMPLE_COUNT 9
#define PCF_SAMPLERS SAMPLE_COUNT
#define BLOCK_SEARCH_COUNT SAMPLE_COUNT
#define NUM_RINGS 11

#define EPS 1e-4
#define DEPTH_BIAS 0.0005
#define PI 3.141592653589793
#define PI2 6.283185307179586

layout(set = 0, binding = 0) uniform PerframeData {
  mat4 proj_view_mat;
  mat4 proj_mat;
  mat4 view_mat;
  mat4 cascade_proj_view_mats[3];
  vec4 depth_splits;
  vec3 camera_pos_ws;
  // float padding_1;
  vec3 sun_ray_dir;
  // float padding_2;
  vec3 sun_pos_ws;
  // float padding_3;
}
perframe_data;

layout(set = 1, binding = 0) uniform sampler2D GPositionTex;
layout(set = 1, binding = 1) uniform sampler2D GNormalTex;
layout(set = 1, binding = 2) uniform sampler2D GAlbedoTex;
// camera projected depth, need divide w.
layout(set = 1, binding = 3) uniform sampler2D GDepth;
layout(set = 1, binding = 4) uniform sampler2D GPBRMaterial;

layout(set = 2, binding = 0) uniform sampler2DArray cascade_shadowmaps;

// sampler ssao factor,  frag which have great posibility for occlusion has a
// grearter val.
layout(set = 3, binding = 0) uniform sampler2D SSAOTex;

layout(location = 0) in vec2 in_uv;

layout(location = 0) out vec4 out_color;

float rand_2to1(vec2 uv) {
  // [0, 1]
  const float a = 12.9898, b = 78.233, c = 43758.5453;
  float dt = dot(uv.xy, vec2(a, b)), sn = mod(dt, PI);
  return fract(sin(sn) * c);
}

// data come form BLS V8 shader.
vec2 shadowOffsets[9] =
    vec2[9](vec2(0.0, 0.0), vec2(0.0, 1.0), vec2(0.7, 0.7), vec2(1.0, 0.0),
            vec2(0.7, -0.7), vec2(0.0, -1.0), vec2(-0.7, -0.7), vec2(-1.0, 0.0),
            vec2(-0.7, 0.7));

vec2 poissonDisk[SAMPLE_COUNT];

void initPoissonDiskSamples(const in vec2 randomSeed) {
  float ANGLE_STEP = PI2 * float(NUM_RINGS) / float(SAMPLE_COUNT);
  float INV_NUM_SAMPLES = 1.0 / float(SAMPLE_COUNT);

  float angle = rand_2to1(randomSeed) * PI2;
  float radius = INV_NUM_SAMPLES;
  float radiusStep = radius;

  for (int i = 0; i < SAMPLE_COUNT; i++) {
    poissonDisk[i] = vec2(cos(angle), sin(angle)) * pow(radius, 0.75);
    radius += radiusStep;
    angle += ANGLE_STEP;
  }
}

// wired???
float PCF_Filter(const in vec2 uv, float z_receiver, float radius,
                 uint cascade_idx) {
  float sum = 0.0;
  for (uint i = 0; i < PCF_SAMPLERS; ++i) {
    float depth =
        texture(cascade_shadowmaps,
                vec3(uv + shadowOffsets[cascade_idx] * radius, cascade_idx))
            .r;
    if (z_receiver < depth) sum += 1.0;
  }

  return sum / PCF_SAMPLERS;
}

float findBlocker(const in vec2 uv, const float z_receiver, uint cascade_idx) {
  // float radius = LIGHT_SIZE_UV * (z_receiver - NEAR_PLANE) / z_receiver;
  float radius = 1.0 / 2048.0;
  float blocker_depth_sum = 0.0;
  uint blocker_num = 0;
  for (uint i = 0; i < BLOCK_SEARCH_COUNT; ++i) {
    float depth = texture(cascade_shadowmaps,
                          vec3(uv + shadowOffsets[i] * radius, cascade_idx))
                      .r;
    if (depth < z_receiver) {
      blocker_depth_sum += depth;
      ++blocker_num;
    }
  }

  return blocker_num == 0 ? -1.0 : blocker_depth_sum / blocker_num;
}

float SimplePCF(const in vec2 uv, float z_receiver, const uint cascade_idx) {
  float sum = 0.0;
  float scale = 1.0 / 2048.0;
  for (int i = -1; i <= 1; ++i) {
    for (int j = -1; j <= 1; ++j) {
      float depth = texture(cascade_shadowmaps,
                            vec3(uv + scale * vec2(i, j), cascade_idx))
                        .r;
      if (depth > z_receiver - DEPTH_BIAS) {
        sum += 1.0;
      }
    }
  }
  return sum / 9.0;
}

// PCSS, para z_receiver without bias
float PCSS(const vec2 uv, float z_receiver, const uint cascade_idx) {
  // test
  // float depth = texture(cascade_shadowmaps, vec3(uv, cascade_idx)).r;
  // if (depth < z_receiver - DEPTH_BIAS) {
  //   return 0.2;
  // } else {
  //   return 1.0;
  // }
  // end test
  float shadow = 1.0;
  z_receiver -= DEPTH_BIAS;
  initPoissonDiskSamples(uv);
  // 1. blocker
  float avg_block_depth = findBlocker(uv, z_receiver, cascade_idx);
  // if (avg_block_depth == -1.0) return 1.0;

  // 2. penumbra size
  // float penumbra_ratio = (z_receiver - avg_block_depth) / avg_block_depth;
  float penumbra_sz = (z_receiver - avg_block_depth) / avg_block_depth * 5.0;
  // 3. pcf
  // float pcf_radius = penumbra_ratio * LIGHT_SIZE_UV * NEAR_PLANE /
  // z_receiver;
  float pcf_radius = 1.0 / 2048.0;
  // return PCF_Filter(uv, z_receiver, pcf_radius, cascade_idx);
  return SimplePCF(uv, z_receiver, cascade_idx);
}

void main() {
  vec3 frag_position_vs = texture(GPositionTex, in_uv).xyz;
  vec4 frag_albedo = texture(GAlbedoTex, in_uv).rgba;
  vec3 frag_normal = texture(GNormalTex, in_uv).xyz;  // already normalized

  // view space
  // vec4 frag_pos_vs =
  //     perframe_data.camera_view_matrix * vec4(frag_position_ws, 1.0);
  // vec2 uv_ls = frag_position_vs.xy * 0.5 + 0.5;
  // float frag_depth = frag_position_vs.z;

  // // unpack split distance.
  // float split_distance[3];
  // split_distance[0] = perframe_data.depth_splits.x;
  // split_distance[1] = perframe_data.depth_splits.y;
  // split_distance[2] = perframe_data.depth_splits.z;
  // // vs: view space.
  // float frag_depth_vs =
  //     (perframe_data.camera_view_matrix * vec4(frag_position_ws, 1.0)).z;

  // uint cascade_idx = 0;
  // for (uint i = 0; i < 3 - 1; ++i) {
  //   if (frag_depth_vs > split_distance[i]) {
  //     cascade_idx = i + 1;
  //   } else {
  //     break;
  //   }
  // }

  // // shadow calculate
  // vec4 frag_pos_ls = perframe_data.cascade_proj_view_matrices[cascade_idx] *
  //                    vec4(frag_position_ws, 1.0);
  // vec2 shadow_coord = frag_pos_ls.st * 0.5 + 0.5;

  // float visibility = PCSS(shadow_coord, frag_pos_ls.z, cascade_idx);

  // vec3 mix_color = vec3(1.0, 1.0, 1.0);
  // if (cascade_idx == 0) {
  //   mix_color = vec3(1.0, 0.0, 0.0);
  // } else if (cascade_idx == 1) {
  //   mix_color = vec3(0.0, 1.0, 0.0);
  // } else if (cascade_idx == 2) {
  //   mix_color = vec3(0.0, 0.0, 1.0);
  // }

  // visibility = SimplePCF(shadow_coord, frag_pos_ls.z);
  // vec3 color = frag_albedo.rgb * visibility;
  // out_color = vec4(mix(color, mix_color, 0.2), 1.0);

  float roughness = texture(GPBRMaterial, in_uv).r;

  float occlusion = texture(SSAOTex, in_uv).r;
  out_color = vec4(frag_albedo.rgb * (1.0 - occlusion), 1.0);
  out_color = vec4(frag_albedo.rgba);

  out_color = vec4(vec3(roughness), 1.0);

  // out_color = vec4(vec3(occlusion), 1.0);
}
