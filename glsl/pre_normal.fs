#version 130
#extension GL_ARB_explicit_attrib_location : enable
#extension GL_ARB_shader_storage_buffer_object : enable

noperspective in vec2 pass_TexCoord;

uniform uint layer;
uniform sampler2DArray kinect_depths;
uniform vec2 texSizeInv;

uniform sampler3D[5] cv_xyz;
uniform sampler3D[5] cv_uv;

#include </inc_bbox_test.glsl>
// uniform uint num_bricks;
layout (std430, binding = 3) buffer Bricks {
  float brick_size;
  uvec3 resolution;
  uint[] bricks;
};

layout(location = 0) out vec3 out_Normal;

void mark_brick(in vec3 pos) {
  pos -= bbox_min;
  uvec3 index = uvec3(floor(pos / brick_size));
  bricks[index.z * resolution.y * resolution.x + index.y * resolution.x + index.x] = 1u;
}

bool is_outside(float d){
  return (d <= 0.0f) || (d >= 1.0f);
}

vec3 calculate_normal(const in vec2 tex_pos) {
  float depth = texture(kinect_depths, vec3(tex_pos, layer)).r;
  if(is_outside(depth)) {
    return vec3(0.0f);
  }
  vec3 world = texture(cv_xyz[layer], vec3(tex_pos, depth)).xyz;
  mark_brick(world);
  // the valid range scales with depth
  // float dist_range_max = 1.0f / depth;
  float dist_range_max = 0.009f;

  vec2 tex_t = tex_pos + vec2(0.0f, texSizeInv.y);
  vec2 tex_b = tex_pos - vec2(0.0f, texSizeInv.y);
  vec2 tex_l = tex_pos - vec2(texSizeInv.x, 0.0f);
  vec2 tex_r = tex_pos + vec2(texSizeInv.x, 0.0f);

  float depth_t = texture(kinect_depths, vec3(tex_t, layer)).r;
  if(depth_t < 0.0f || abs(depth - depth_t) > dist_range_max) depth_t = 4.5f;
  // if(depth_t < 0.0f || abs(depth - depth_t) > dist_range_max) depth_t = depth;
  float depth_b = texture(kinect_depths, vec3(tex_b, layer)).r;
  if(depth_b < 0.0f || abs(depth - depth_b) > dist_range_max) depth_b = 4.5f;
  // if(depth_b < 0.0f || abs(depth - depth_b) > dist_range_max) depth_b = depth;
  float depth_l = texture(kinect_depths, vec3(tex_l, layer)).r;
  if(depth_l < 0.0f || abs(depth - depth_l) > dist_range_max) depth_l = 4.5f;
  // if(depth_l < 0.0f || abs(depth - depth_l) > dist_range_max) depth_l = depth;
  float depth_r = texture(kinect_depths, vec3(tex_r, layer)).r;
  if(depth_r < 0.0f || abs(depth - depth_r) > dist_range_max) depth_r = 4.5f;
  // if(depth_r < 0.0f || abs(depth - depth_r) > dist_range_max) depth_r = depth;
  vec3 world_t = texture(cv_xyz[layer], vec3(tex_t, depth_t)).xyz;
  vec3 world_b = texture(cv_xyz[layer], vec3(tex_b, depth_b)).xyz;
  vec3 world_l = texture(cv_xyz[layer], vec3(tex_l, depth_l)).xyz;
  vec3 world_r = texture(cv_xyz[layer], vec3(tex_r, depth_r)).xyz;

  return normalize(cross(world_b - world_t, world_l - world_r));
}

void main(void) {
  out_Normal = calculate_normal(pass_TexCoord);
}
