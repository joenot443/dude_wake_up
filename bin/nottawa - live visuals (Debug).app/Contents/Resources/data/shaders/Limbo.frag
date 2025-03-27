#version 150

uniform float time;
uniform float colorWarp;
uniform vec2 dimensions;
in vec2 coord;
out vec4 outputColor;

void main(void) {

  float scale = dimensions.y / 100.0;
  float ring = 50.0;
  float radius = dimensions.x * 1.0;
  float gap = scale * .5;
  vec2 pos = gl_FragCoord.xy - dimensions.xy * .5;

  float d = length(pos);

  // Create the wiggle
  d += (sin(pos.y * 0.25 / scale + time) *
        sin(pos.x * 0.25 / scale + time * .5)) *
       scale * 2.0;

  // Compute the distance to the closest ring
  float v = mod(d + radius / (ring * 2.0), radius / ring);
  v = abs(v - radius / (ring * 2.0));

  v = clamp(v - gap, 0.0, 1.0);

  d /= radius;
  vec3 m = fract((d - 1.0) * vec3(ring * -.5, -ring, ring * .25) * 0.5 * colorWarp);

  outputColor = vec4(m * v, 1.0);
}
