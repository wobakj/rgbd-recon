#version 330
// vertex attributes of vao
layout(location=0) in vec3 in_Position;
layout(location=1) in vec3 in_Normal;
layout(location=2) in vec2 in_Texcoord;
layout(location=3) in vec4 in_Tangent;

//Matrix Uniforms as specified with glUniformMatrix4fv
uniform mat4 ModelMatrix;
uniform mat4 gl_ModelViewMatrix;
uniform mat4 gl_ProjectionMatrix;

out vec3 pass_Position;
out vec3 pass_Normal;
out vec2 pass_Texcoord;
out vec4 pass_Tangent;

void main() {
  gl_Position = gl_ProjectionMatrix * gl_ModelViewMatrix * ModelMatrix * vec4(in_Position, 1.0f);
  
  pass_Position = (gl_ModelViewMatrix * ModelMatrix * vec4(in_Position, 1.0f)).xyz;
  // pass_Normal = normalize(((gl_ModelViewMatrix * ModelMatrix) * vec4(in_Normal, 0.0f)).xyz);
  pass_Normal = in_Normal;
  pass_Tangent.xyz = normalize(((gl_ModelViewMatrix * ModelMatrix) * vec4(in_Tangent.xyz, 0.0f)).xyz);
  pass_Tangent.w = in_Tangent.w;
  pass_Texcoord = in_Texcoord;
}
