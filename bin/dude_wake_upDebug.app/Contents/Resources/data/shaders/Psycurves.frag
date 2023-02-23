#version 150

uniform sampler2D tex;
uniform float time;
uniform vec2 dimensions;
in vec2 coord;
out vec4 outputColor;

void main(void) {
  vec2 uv = (coord.xy - dimensions.xy * .5) / dimensions.y;

  uv *= mat2(.707, -.707, .707, .707);
  uv *= 15.;

  vec2 gv = fract(uv) - .5;
  vec2 id = floor(uv);

  vec3 m = vec3(.5, 1, 0);
  float t;
  for (float y = -1.; y <= 1.; y++) {
    for (float x = -1.; x <= 1.; x++) {
      vec2 offs = vec2(x, y);

      t = -time + length(id - offs) * .2;
      float r = mix(.4, 1.5, sin(t) * .5 + .5);
      float c = smoothstep(r, r * .9, length(gv + offs));
      m.zyx = m.yxz * (1. - c) + c * (1. - m.xzy);
    }
  }

  outputColor = vec4(m, 1);
}
