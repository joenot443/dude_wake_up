#version 150

uniform sampler2D tex;
uniform float shape;
uniform vec2 dimensions;
uniform float time;
in vec2 coord;
out vec4 outputColor;

#define steps 2.

void main() {
  vec2 uv = coord;
  vec4 c = texture(tex, uv);
  float g = max(c.r, max(c.g, c.b)) * steps;
  float fuck = 345.678 + shape * dimensions.x;
  float f = mod((uv.x + uv.y + 500.) * fuck, 1.);
  if (mod(g, 1.0) > f)
    c.r = ceil(g);
  else
    c.r = floor(g);
  c.r /= steps;
  outputColor = c.rrra;
}
