#version 150

uniform sampler2D tex;
uniform sampler2D maskTex;
uniform vec2 dimensions;
uniform float time;
uniform float blend;
in vec2 coord;
out vec4 outputColor;

void main() {

  vec4 tex_color = texture(tex, coord);
  vec4 mask_color = texture(maskTex, coord);

  if (mask_color.a < 0.5) {
    outputColor = vec4(0.0);
    return;
  }
  
  outputColor = vec4(tex_color.rgb, blend);
}
