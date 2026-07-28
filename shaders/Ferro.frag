#version 150

// Ferro — ferrofluid / cosmic ink smoke. A dot-noise fbm is domain-warped twice into
// billowing ink, a third RIDGED octave carves fine filament hair into the density, and
// the field is colored with a disciplined magenta<->teal cosine sheen (density-biased
// hue, amber-proof). Deep graded voids give the central well mass; a gradient-driven
// specular adds a wet liquid-metal crest highlight; vignette + edge desaturation give a
// cinematic center of mass. Generated from scratch and art-directed via two review passes.

uniform sampler2D tex;
uniform vec2 dimensions;
uniform float time;        // speed-scaled accumulated phase (integrated on CPU)
uniform float scale;       // zoom of the smoke
uniform float warp;        // domain-warp strength (billow turbulence)
uniform float detailAmount;// filament-hair carve strength
uniform float hueShift;    // palette phase offset
uniform float saturation;  // global color vividness
uniform float brightness;  // exposure

in vec2 coord;
out vec4 outputColor;

float dot_noise(vec3 p){
  const float PHI = 1.618033988;
  const mat3 G = mat3(-0.571464913, 0.814921382, 0.096597072,
                      -0.278044873, -0.303026659, 0.911518454,
                       0.772087367, 0.494042493, 0.399753815);
  return dot(cos(G*p), sin(PHI*p*G));
}
float fbm(vec3 p){
  float v = 0.0, a = 0.5;
  for(int i=0;i<6;i++){ v += a*dot_noise(p)/3.0; p *= 1.92; a *= 0.5; }
  return v;
}
void main(){
  vec2 uv = (2.0*coord - dimensions)/dimensions.y;
  vec3 q = vec3(uv*scale, time*0.06);
  float w1 = fbm(q);
  float w2 = fbm(q + warp*vec3(w1*1.5, w1*1.2, 0.0) + vec3(3.1, 1.7, 0.0));
  float d  = fbm(q + warp*vec3(w2*2.0) + vec3(0.0, time*0.10, 0.0));

  // third ridged octave injects fine filament hair into the density itself
  float w3 = fbm(q*3.6 + vec3(w2*0.8) + vec3(0.0, 0.0, time*0.18));
  float ridge = pow(1.0 - abs(w3), 4.0);
  d = mix(d, d + (ridge - 0.45)*0.5, clamp(0.55*detailAmount, 0.0, 1.0));

  float ink = 1.0 - abs(d);
  ink = pow(ink, 2.6);

  float fine = 1.0 - abs(fbm(q*2.8 + vec3(w2*1.8) + vec3(0.0, time*0.12, 2.0)));
  fine = pow(max(fine, 0.0), 2.0);
  float detail = 0.6 + 0.8*fine;
  float glow = smoothstep(0.16, 1.0, ink) * detail;

  // disciplined magenta<->teal palette, density-biased hue, amber-proof
  float t = w2*0.6 + d*0.4 + time*0.04 + (ink - 0.3)*0.25 + hueShift;
  vec3 sheen = vec3(0.50, 0.12, 0.50)
             + vec3(0.48, 0.20, 0.42) * cos(6.2831853*(vec3(0.95, 0.80, 1.05)*t
                                            + vec3(0.00, 0.55, 0.70)));

  vec3 col = sheen * glow;
  col += sheen * pow(ink, 5.0) * 0.8;             // bright filament cores

  // wet specular: gradient-driven so only steep crests catch light
  float e = 0.6/dimensions.y;
  float dx = fbm(q + vec3(e,0,0)) - fbm(q - vec3(e,0,0));
  float dy = fbm(q + vec3(0,e,0)) - fbm(q - vec3(0,e,0));
  float grad = length(vec2(dx, dy)) * 40.0;
  vec3 nrm = normalize(vec3(dx, dy, 0.04 + 0.5/(1.0 + grad)));
  float spec = pow(max(nrm.z, 0.0), 24.0) * smoothstep(0.45, 0.85, ink);
  col += vec3(0.6, 0.95, 0.85) * spec * 0.6;

  // deepen voids — graded recede into indigo-black + crush the deep cores
  float voidness = 1.0 - smoothstep(0.0, 0.35, glow);
  col = mix(col, vec3(0.01, 0.015, 0.035), voidness*0.85);
  col *= mix(0.35, 1.0, smoothstep(0.0, 0.25, ink));

  // vignette + edge desaturation for cinematic depth
  float vig = smoothstep(1.5, 0.2, length(uv));
  col *= mix(0.55, 1.0, vig);
  col = mix(vec3(dot(col, vec3(0.333))), col, mix(0.82, 1.0, vig));

  // global vividness control
  col = mix(vec3(dot(col, vec3(0.333))), col, saturation);

  col = 1.0 - exp(-col * 1.7 * brightness);
  outputColor = vec4(col, 1.0);
}
