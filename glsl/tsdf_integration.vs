#version 430

in vec3 in_Position; 
// input
uniform sampler2DArray kinect_colors;
uniform sampler2DArray kinect_depths;
uniform sampler2DArray kinect_qualities;
uniform sampler2DArray kinect_normals;
uniform sampler2DArray kinect_silhouettes;

// calibration
uniform sampler3D[5] cv_xyz_inv;

layout(rg32f) uniform image3D volume_tsdf;

uniform float limit;
uniform uint num_kinect;
uniform uvec3 res_tsdf;

void main() {
  vec2 weighted_tsd = vec2(limit, 0.0);
  // coordinates must be even pixels
  ivec3 ipos_vol = ivec3(in_Position * res_tsdf);
  if (num_kinect > 0) {
    weighted_tsd = imageLoad(volume_tsdf, ipos_vol).rg;
  }

  float total_weight = 0;
  vec3 pos_calib = texture(cv_xyz_inv[num_kinect], in_Position).xyz;
  float silhouette = texture(kinect_silhouettes, vec3(pos_calib.xy, float(num_kinect))).r;
  if (silhouette < 1.0f) {
    // no write yet -> voxel outside of surface
    if (weighted_tsd.r >= limit) {
      weighted_tsd.r = -limit;
    }
  }
  float depth = texture(kinect_depths, vec3(pos_calib.xy, float(num_kinect))).r;
  float sdist = pos_calib.z - depth;
  if (sdist <= -limit ) {
    weighted_tsd.r = -limit;
    // break;
  }
  else if (sdist >= limit ) {
    // do nothing
  }
  else {
    float weight = texture(kinect_qualities, vec3(pos_calib.xy, float(num_kinect))).r;
    weighted_tsd.r = (weighted_tsd.r * weighted_tsd.g + weight * sdist) / (weighted_tsd.g + weight);
    weighted_tsd.g += weight;
  }

  imageStore(volume_tsdf, ipos_vol, vec4(weighted_tsd, 0.0f, 0.0f));
}