#version 150

uniform sampler2D tex;
uniform vec2 dimensions;
uniform float time;
uniform vec4 color;
in vec2 coord;
out vec4 outputColor;

void main() {
  vec4 frag_tex = texture(tex, coord);
  
  if (frag_tex.a < 0.01) {
    outputColor = color;
  } else {
    outputColor = frag_tex;
  }
}
