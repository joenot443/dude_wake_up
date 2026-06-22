#version 150

// Gemfield — glowing stained-glass / inflated-gem cells. Generated from scratch
// via the new-shader-from-scratch skill. An animated Voronoi diagram: each cell
// is shaded brighter toward its center (a glass-pebble bevel for depth), the
// borders glow as tinted seams, and a radial ripple makes the field breathe.

uniform sampler2D tex;
uniform vec2 dimensions;
uniform float time;       // speed-scaled accumulated phase (integrated on CPU)
uniform float scale;      // cell density (higher = smaller gems)
uniform float seamGlow;   // brightness of the glowing cell borders
uniform float ripple;     // strength of the breathing radial ripple
uniform float hueShift;   // palette phase offset
uniform float brightness; // exposure

in vec2 coord;
out vec4 outputColor;

#define PI 3.14159265

vec2 hash2(vec2 p){
  p = vec2(dot(p, vec2(127.1, 311.7)), dot(p, vec2(269.5, 183.3)));
  return fract(sin(p) * 43758.5453);
}

vec3 palette(float t){
  return 0.5 + 0.5 * cos(2.0 * PI * (vec3(1.0) * t + vec3(0.0, 0.33, 0.6)));
}

void main(){
  vec2 uv = (2.0 * coord - dimensions) / dimensions.y;
  vec2 p = uv * scale;
  vec2 g = floor(p), f = fract(p);

  float d1 = 1e5, d2 = 1e5;   // nearest and 2nd-nearest site distances
  vec2 closest = g;
  for (int y = -1; y <= 1; y++)
    for (int x = -1; x <= 1; x++) {
      vec2 o = vec2(x, y);
      vec2 pt = o + 0.5 + 0.45 * sin(time * 0.5 + 6.2831 * hash2(g + o));
      float d = length(f - pt);
      if (d < d1) { d2 = d1; d1 = d; closest = g + o; }
      else if (d < d2) { d2 = d; }
    }

  float edge = d2 - d1;                       // distance to the cell border
  float seam = exp(-edge * 7.0);              // soft glowing seam
  float cellId = hash2(closest).x;
  float shade = smoothstep(0.9, 0.0, d1);     // glass-pebble: bright cell center
  float rip = sin(length(uv) * 3.0 - time * 1.8) * 0.5 + 0.5;

  vec3 base = palette(cellId + time * 0.04 + hueShift);
  // ripple scales the breathing depth; at ripple=1 this is the original 0.6+0.6*rip
  vec3 col = base * (0.12 + 0.8 * shade) * (0.6 + 0.6 * ripple * rip);
  col += base * seam * seamGlow;              // tinted glowing seams

  col = 1.0 - exp(-col * brightness);          // soft tone map
  outputColor = vec4(col, 1.0);
}
