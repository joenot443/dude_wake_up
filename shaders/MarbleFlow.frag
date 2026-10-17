#version 150

// MarbleFlow — liquid dot-noise marble / flowing ink. Generated from scratch via
// the new-shader-from-scratch skill. Domain-warps a cheap aperiodic "dot noise"
// fbm (XorDev) twice, then colors the folded field with a cosine palette for a
// soft, chromatically-rich marbling that drifts and folds.

uniform sampler2D tex;
uniform vec2 dimensions;
uniform float time;       // speed-scaled accumulated phase (integrated on CPU)
uniform float scale;      // zoom of the marble
uniform float warp;       // domain-warp strength (more = more turbulent folds)
uniform float hueShift;   // palette phase offset
uniform float brightness; // exposure

in vec2 coord;
out vec4 outputColor;

vec3 palette(float t){
  return 0.5 + 0.5 * cos(6.2831853 * (vec3(1.0) * t + vec3(0.0, 0.5, 0.7)));
}

// XorDev dot noise — aperiodic, no hashing. Returns [-3, 3].
float dn(vec3 p){
  const float PHI = 1.618033988;
  const mat3 G = mat3(-0.571464913, 0.814921382, 0.096597072,
                      -0.278044873, -0.303026659, 0.911518454,
                       0.772087367, 0.494042493, 0.399753815);
  return dot(cos(G * p), sin(PHI * p * G));
}
float dfbm(vec3 p){
  float v = 0.0, a = 0.5;
  for (int i = 0; i < 5; i++){ v += a * dn(p) / 3.0; p *= 1.9; a *= 0.5; }
  return v;
}

void main(){
  vec2 uv = (2.0 * coord - dimensions) / dimensions.y;
  vec3 b = vec3(uv * scale, time * 0.1);

  // Two-step domain warp: fold the field through itself for marbling.
  float q = dfbm(b);
  float r = dfbm(b + 2.0 * warp * vec3(q, q * 0.7, 0.0) + vec3(5.2, 1.3, 0.0));
  float f = dfbm(b + 3.0 * warp * vec3(r));

  vec3 c = palette(f + 0.4 * r + time * 0.04 + hueShift) * (0.6 + 0.6 * f);
  c = 1.0 - exp(-c * brightness); // soft tone map
  outputColor = vec4(c, 1.0);
}
