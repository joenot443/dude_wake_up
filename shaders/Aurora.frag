#version 150

uniform sampler2D tex;
uniform vec2 dimensions;
uniform float time;
uniform float speed;
uniform float intensity;
uniform float waveHeight;
uniform float colorShift;
uniform float bandCount;
uniform float noiseScale;
in vec2 coord;
out vec4 outputColor;

// Simplex-style noise
vec3 mod289(vec3 x) { return x - floor(x * (1.0 / 289.0)) * 289.0; }
vec2 mod289(vec2 x) { return x - floor(x * (1.0 / 289.0)) * 289.0; }
vec3 permute(vec3 x) { return mod289(((x * 34.0) + 1.0) * x); }

float snoise(vec2 v) {
  const vec4 C = vec4(0.211324865405187, 0.366025403784439,
                      -0.577350269189626, 0.024390243902439);
  vec2 i  = floor(v + dot(v, C.yy));
  vec2 x0 = v - i + dot(i, C.xx);
  vec2 i1 = (x0.x > x0.y) ? vec2(1.0, 0.0) : vec2(0.0, 1.0);
  vec4 x12 = x0.xyxy + C.xxzz;
  x12.xy -= i1;
  i = mod289(i);
  vec3 p = permute(permute(i.y + vec3(0.0, i1.y, 1.0)) + i.x + vec3(0.0, i1.x, 1.0));
  vec3 m = max(0.5 - vec3(dot(x0, x0), dot(x12.xy, x12.xy), dot(x12.zw, x12.zw)), 0.0);
  m = m * m;
  m = m * m;
  vec3 x = 2.0 * fract(p * C.www) - 1.0;
  vec3 h = abs(x) - 0.5;
  vec3 ox = floor(x + 0.5);
  vec3 a0 = x - ox;
  m *= 1.79284291400159 - 0.85373472095314 * (a0 * a0 + h * h);
  vec3 g;
  g.x = a0.x * x0.x + h.x * x0.y;
  g.yz = a0.yz * x12.xz + h.yz * x12.yw;
  return 130.0 * dot(m, g);
}

float fbm(vec2 p, float t) {
  float f = 0.0;
  float amp = 0.5;
  for (int i = 0; i < 5; i++) {
    f += amp * snoise(p + t * 0.1);
    p *= 2.1;
    amp *= 0.5;
  }
  return f;
}

void main() {
  vec2 uv = coord;
  float t = time * speed;

  // Aurora lives in the upper portion of the screen
  float y = uv.y;

  // Multiple flowing aurora bands
  float aurora = 0.0;
  for (float i = 0.0; i < 4.0; i++) {
    if (i >= bandCount) break;
    float offset = i * 0.15 + 0.3;
    float n = fbm(vec2(uv.x * noiseScale + i * 1.7, t * 0.3 + i * 0.5), t);
    float wave = offset + n * waveHeight;
    float dist = abs(y - wave);
    float band = exp(-dist * (8.0 - intensity * 4.0)) * intensity;
    aurora += band;
  }

  // Aurora colors - shifting greens, teals, and purples
  float hue = colorShift + uv.x * 0.1 + fbm(vec2(uv.x * 2.0, t * 0.2), t) * 0.15;
  vec3 col1 = vec3(0.1, 1.0, 0.4);  // green
  vec3 col2 = vec3(0.1, 0.6, 1.0);  // teal
  vec3 col3 = vec3(0.6, 0.1, 1.0);  // purple

  float h = fract(hue);
  vec3 auroraColor;
  if (h < 0.33) {
    auroraColor = mix(col1, col2, h * 3.0);
  } else if (h < 0.66) {
    auroraColor = mix(col2, col3, (h - 0.33) * 3.0);
  } else {
    auroraColor = mix(col3, col1, (h - 0.66) * 3.0);
  }

  // Blend aurora with input texture
  vec4 texColor = texture(tex, coord);
  vec3 glow = auroraColor * aurora;
  vec3 result = texColor.rgb + glow;

  outputColor = vec4(result, 1.0);
}
