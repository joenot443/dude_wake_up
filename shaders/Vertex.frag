#version 150

#define S(a, b, t) smoothstep(a, b, t)

// Uniforms
uniform sampler2D tex;
uniform vec2 dimensions;
uniform vec3 color;
uniform float time;

// New uniforms for user control
uniform float layerCount;      // Number of net layers
uniform float netSpeed;        // Net animation speed
uniform float sparkleIntensity;// Sparkle effect strength
uniform float lineThickness;   // Net line thickness
uniform float netRotation;     // Manual net rotation
uniform float glowStrength;    // Glow effect strength
uniform int colorMode;         // Colorization mode
uniform float pulseFrequency;  // Node pulse frequency
uniform float netScale;        // Net pattern scale
uniform float edgeFade;        // Edge vignette strength

// Varyings
in vec2 coord;
out vec4 outputColor;

// Hash function for pseudo-randomness
float hash21(vec2 p) {
  vec3 a = fract(vec3(p.xyx) * vec3(213.897, 653.453, 253.098));
  a += dot(a, a.yzx + 79.76);
  return fract((a.x + a.y) * a.z);
}

// Get animated position for a grid point
vec2 getPos(vec2 id, vec2 offset, float t) {
  float n = hash21(id + offset);
  float n1 = fract(n * 10.);
  float n2 = fract(n * 100.);
  float a = t + n;
  return offset + vec2(sin(a * n1), cos(a * n2)) * 0.4;
}

// Distance from point p to line segment ab
float distToLine(vec2 a, vec2 b, vec2 p) {
  vec2 pa = p - a, ba = b - a;
  float h = clamp(dot(pa, ba) / dot(ba, ba), 0., 1.);
  return length(pa - ba * h);
}

// Draw a line with smooth edge
float drawLine(vec2 a, vec2 b, vec2 uv) {
  float r1 = 0.04 * lineThickness;
  float r2 = 0.01 * lineThickness;
  float d = distToLine(a, b, uv);
  float d2 = length(a - b);
  float fade = S(1.5, 0.5, d2);
  fade += S(0.05, 0.02, abs(d2 - 0.75));
  return S(r1, r2, d) * fade;
}

// Draw a single net layer
float netLayer(vec2 st, float layerIdx, float t) {
  vec2 id = floor(st) + layerIdx;
  st = fract(st) - 0.5;

  vec2 points[9];
  int i = 0;
  for (float y = -1.; y <= 1.; y++) {
    for (float x = -1.; x <= 1.; x++) {
      points[i++] = getPos(id, vec2(x, y), t);
    }
  }

  float m = 0.;
  float sparkle = 0.;

  for (int j = 0; j < 9; j++) {
    m += drawLine(points[4], points[j], st);
    float d = length(st - points[j]);
    float s = (0.005 / (d * d));
    s *= S(1., 0.7, d);
    float pulse = sin((fract(points[j].x) + fract(points[j].y) + t) * pulseFrequency) * 0.4 + 0.6;
    pulse = pow(pulse, 20.);
    s *= pulse;
    sparkle += s * sparkleIntensity;
  }

  // Draw cross lines
  m += drawLine(points[1], points[3], st);
  m += drawLine(points[1], points[5], st);
  m += drawLine(points[7], points[5], st);
  m += drawLine(points[7], points[3], st);

  float sPhase = (sin(t + layerIdx) + sin(t * 0.1)) * 0.25 + 0.5;
  sPhase += pow(sin(t * 0.1) * 0.5 + 0.5, 50.) * 5.;
  m += sparkle * sPhase;

  return m;
}

void main() {
  // Centered and normalized coordinates
  vec2 uv = (coord - dimensions.xy * 0.5) / dimensions.y;
  vec2 mouse = vec2(0.1, 0.1); // Unused, but kept for future use

  float t = time * netSpeed;
  float s = sin(t + netRotation);
  float c = cos(t + netRotation);
  mat2 rot = mat2(c, -s, s, c);
  vec2 st = uv * rot;
  mouse *= rot * 2.;

  float m = 0.;
  for (float i = 0.; i < 1.; i += 1. / layerCount) {
    float z = fract(t + i);
    float size = mix(15. * netScale, 1. * netScale, z);
    float fade = S(0., 0.6, z) * S(1., 0.8, z);
    m += fade * netLayer(st * size - mouse * z, i, time);
  }

  float fft = texelFetch(tex, ivec2(0.7, 0), 0).x;
  float glow = -uv.y * fft * glowStrength;

  // Color selection
  vec3 baseCol;
  if (colorMode == 0) {
    baseCol = (color.x != 0.0 && color.y != 0.0 && color.z != 0.0)
      ? color
      : vec3(s, cos(t * 0.4), -sin(t * 0.24)) * 0.4 + 0.6;
  } else if (colorMode == 1) {
    baseCol = vec3(sin(t), cos(t * 0.5), sin(t * 0.2)) * 0.5 + 0.5;
  } else {
    baseCol = vec3(1.0, 1.0, 1.0);
  }

  vec3 col = baseCol * m;
  col += baseCol * glow;

  // Edge fade
  col *= 1. - dot(uv, uv) * edgeFade;
  t = mod(time, 230.);
  col *= S(0., 20., t) * S(224., 200., t);

  outputColor = vec4(col, 1);
}
