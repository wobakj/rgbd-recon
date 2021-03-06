#version 130

noperspective in vec2 pass_TexCoord;

uniform sampler2D texture_handle;
uniform sampler2DArray texture_array;
uniform uint layer;
uniform uint mode;

out vec4 out_FragColor;

void main() {

  vec3 coords = vec3(pass_TexCoord, layer);
  if (mode == 0u) {
    out_FragColor = vec4(texture(texture_handle, pass_TexCoord).rgb, 1.0f);
  }
  else {
    out_FragColor = vec4(texture(texture_array, coords).rgb, 1.0f);
  }
}
