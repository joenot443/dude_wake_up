#version 150

// Silk — iridescent draped-satin flow field. Generated from scratch via the
// new-shader-from-scratch skill. Advects each pixel through a noise flow field,
// accumulating soft palette-colored streaks; overlapping streaks read as folded,
// glossy fabric. A tinted sheen highlights the ridges.

uniform sampler2D tex;
uniform vec2 dimensions;
uniform float time;       // speed-scaled accumulated phase (integrated on CPU)
uniform float scale;      // zoom of the flow field
uniform float flow;       // advection step size per iteration
uniform float hueShift;   // palette phase offset
uniform float sheen;      // strength of the satin highlight on ridges
uniform float brightness; // exposure

in vec2 coord;
out vec4 outputColor;

#define PI 3.14159265

float hash(vec2 p){ return fract(sin(dot(p, vec2(127.1, 311.7))) * 43758.5453); }

float noise(vec2 p){
  vec2 i = floor(p), f = fract(p);
  vec2 u = f * f * (3.0 - 2.0 * f);
  return mix(mix(hash(i), hash(i + vec2(1, 0)), u.x),
             mix(hash(i + vec2(0, 1)), hash(i + vec2(1, 1)), u.x), u.y);
}

vec3 palette(float t){
  return 0.5 + 0.5 * cos(2.0 * PI * (vec3(1.0) * t + vec3(0.2, 0.5, 0.8)));
}

void main(){
  vec2 uv = (2.0 * coord - dimensions) / dimensions.y;
  vec2 p = uv * scale;
  vec3 col = vec3(0.0);
  float h = 0.0;

  // Advect through the flow field, accumulating soft streaks.
  for (int i = 0; i < 32; i++) {
    float n = noise(p * 1.5 + time * 0.18);
    float ang = n * 2.0 * PI * 2.0 + time * 0.1;
    p += vec2(cos(ang), sin(ang)) * flow;
    float s = noise(p * 3.0);
    float streak = exp(-abs(s - 0.5) * 8.0);
    h += streak;
    col += palette(n + time * 0.05 + float(i) * 0.02 + hueShift) * streak * 0.042;
  }

  // Tinted satin sheen on the densest ridges (keeps saturation vs a white sheen).
  col += palette(0.5 + 0.1 * time + hueShift) * smoothstep(0.9, 1.8, h) * sheen;

  col = 1.0 - exp(-col * brightness); // soft tone map
  outputColor = vec4(col, 1.0);
}
