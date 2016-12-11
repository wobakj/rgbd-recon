#version 330
#extension GL_ARB_shader_storage_buffer_object: enable
#extension GL_ARB_arrays_of_arrays: enable

#define BLINN

uniform sampler2D NormalTexture;
uniform sampler2D ColorTexture;

in vec3 pass_Position;
in vec3 pass_Normal;
in vec2 pass_Texcoord;
in vec4 pass_Tangent;

vec3 LightPosition; //diffuse color
vec3 LightDiffuse; //diffuse color
vec3 LightAmbient;        //ambient color
vec3 LightSpecular;       //specular color

// material
const float ks = 0.9f;            // specular intensity
const float n = 20.0f;            //specular exponent 

out vec4 out_Color;

void main() {
  // set light to override value in ssbo
  LightAmbient = vec3(0.25f);

  vec3 diffuseColor = texture(ColorTexture, pass_Texcoord).xyz;
  out_Color = vec4(diffuseColor, 1.0);
  // out_Color = vec4(pass_Normal, 1.0);
  return;
  // read tangent-space normal from texture
  vec3 normal_tangent = texture(NormalTexture, pass_Texcoord).xyz;
  // colors are normalized but x- and y-components can go from -1 to 1
  normal_tangent = normal_tangent * vec3(2.0f, 2.0f, 1.0f) - vec3(1.0f, 1.0f, 0.0f);
  // transform normal from tangent- to view-space
  mat3 tangent_matrix = mat3(pass_Tangent.xyz,
                             normalize(pass_Tangent.w * cross(pass_Normal, pass_Tangent.xyz)),
                             pass_Normal);
  vec3 normal_view = normalize(tangent_matrix * normal_tangent); 
 
  vec3 toLight = normalize(LightPosition - pass_Position);
  float lightAngle = dot(normal_view, toLight);
  // if fragment is not directly lit, use only ambient light
  if (lightAngle <= 0.0f) {
    out_Color = vec4(LightAmbient * diffuseColor, 1.0f);
    return;
  }

  vec3 diffContribution = diffuseColor * max(lightAngle, 0.0f);

  vec3 toViewer = normalize(-pass_Position);
  #ifdef BLINN
    vec3 halfwayVector = normalize(toLight + toViewer);
    float reflectedAngle = dot(halfwayVector, normal_view);
    float specLight = pow(reflectedAngle, n);
  #else
    vec3 reflectedLight = reflect(-toLight, normal_view);
    float reflectedAngle = max(dot(reflectedLight, toViewer), 0.0f);
    float specLight = pow(reflectedAngle, n * 0.25f);
  #endif
  // fade out specular hightlights towards edge of lit region
  float a = (1.0f - lightAngle) * ( 1.0f - lightAngle);
  specLight *= 1.0f - a * a * a;

  out_Color = vec4(LightAmbient * diffuseColor 
                 + LightDiffuse * diffContribution
                 + LightSpecular * ks * specLight, 1.0f);

}