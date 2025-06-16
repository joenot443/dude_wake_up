#version 150

// Uniforms
uniform sampler2D tex;
uniform vec2 dimensions;
uniform float time;
uniform float amount;
// New uniforms for enhanced control
uniform float noiseScale;      // Controls zoom of noise pattern
uniform float warpSpeed;       // Controls animation speed
uniform float colorOffset;     // Shifts colormap input
uniform float complexity;      // Controls pattern complexity
uniform vec2 direction;        // Directional distortion
uniform int colormapType;      // Selects colormap
uniform float edgeSoftness;    // Controls pattern contrast/softness

// Varyings
in vec2 coord;
out vec4 outputColor;

// --- Colormap functions ---
vec4 colormap(float x, int type) {
  // Default colormap (original)
  float r, g, b;
  if (type == 0) {
    // Red
    if (x < 0.0) {
      r = 54.0 / 255.0;
    } else if (x < 20049.0 / 82979.0) {
      r = (829.79 * x + 54.51) / 255.0;
    } else {
      r = 1.0;
    }
    // Green
    if (x < 20049.0 / 82979.0) {
      g = 0.0;
    } else if (x < 327013.0 / 810990.0) {
      g = (8546482679670.0 / 10875673217.0 * x - 2064961390770.0 / 10875673217.0) / 255.0;
    } else if (x <= 1.0) {
      g = (103806720.0 / 483977.0 * x + 19607415.0 / 483977.0) / 255.0;
    } else {
      g = 1.0;
    }
    // Blue
    if (x < 0.0) {
      b = 54.0 / 255.0;
    } else if (x < 7249.0 / 82979.0) {
      b = (829.79 * x + 54.51) / 255.0;
    } else if (x < 20049.0 / 82979.0) {
      b = 127.0 / 255.0;
    } else if (x < 327013.0 / 810990.0) {
      b = (792.0224934136139 * x - 64.36479073560233) / 255.0;
    } else {
      b = 1.0;
    }
    return vec4(r, g, b, 1.0);
  } else if (type == 1) {
    // Rainbow colormap
    return vec4(0.5 + 0.5 * sin(6.2831 * (x + vec3(0.0, 0.33, 0.67))), 1.0);
  } else if (type == 2) {
    // Grayscale
    return vec4(vec3(x), 1.0);
  } else if (type == 3) {
    // Fire colormap
    return vec4(x, x * x, x * x * x, 1.0);
  }
  // Fallback
  return vec4(x, x, x, 1.0);
}

// --- Noise functions ---
float rand(vec2 n) {
  return fract(sin(dot(n, vec2(12.9898, 4.1414))) * 43758.5453);
}

float noise(vec2 p) {
  vec2 ip = floor(p);
  vec2 u = fract(p);
  u = u * u * (3.0 - 2.0 * u);
  float res = mix(
    mix(rand(ip), rand(ip + vec2(1.0, 0.0)), u.x),
    mix(rand(ip + vec2(0.0, 1.0)), rand(ip + vec2(1.0, 1.0)), u.x),
    u.y
  );
  return res * res;
}

// 2D rotation/shear matrix
const mat2 FBM_MAT = mat2(0.80, 0.60, -0.60, 0.80);

// Fractional Brownian Motion with complexity and direction
float fbm(vec2 p, float complexity, vec2 direction) {
  float f = 0.0;
  float c = clamp(complexity, 1.0, 6.0); // Clamp to max 6 octaves
  vec2 dir = direction;
  if (c >= 1.0) { f += amount * 0.500000 * noise(p + dir + time * warpSpeed); p = FBM_MAT * p * 2.02; }
  if (c >= 2.0) { f += amount * 0.031250 * noise(p + dir); p = FBM_MAT * p * 2.01; }
  if (c >= 3.0) { f += amount * 0.250000 * noise(p + dir); p = FBM_MAT * p * 2.03; }
  if (c >= 4.0) { f += amount * 0.125000 * noise(p + dir); p = FBM_MAT * p * 2.01; }
  if (c >= 5.0) { f += amount * 0.062500 * noise(p + dir); p = FBM_MAT * p * 2.04; }
  if (c >= 6.0) { f += amount * 0.015625 * noise(p + dir + sin(time * warpSpeed)); }
  return f / 0.96875;
}

// Nested FBM pattern with noiseScale
float pattern(vec2 p, float complexity, vec2 direction) {
  p *= noiseScale;
  return fbm(p + fbm(p + fbm(p, complexity, direction), complexity, direction), complexity, direction);
}

void main() {
  // Normalize coordinates to [0,1] range
  vec2 uv = coord / dimensions.x;
  // Apply directional distortion
  uv += direction;
  float shade = pattern(uv, complexity, direction);
  // Edge softness/contrast
  float softShade = pow(shade, edgeSoftness);
  // Color offset
  float colorInput = softShade + colorOffset;
  outputColor = colormap(colorInput, colormapType);
}
