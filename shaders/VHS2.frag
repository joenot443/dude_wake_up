#version 150

uniform sampler2D tex;
uniform vec2 dimensions;
uniform float time;
uniform float hsyncJitter;
uniform float blurIntensity;
uniform float jitterIntensity;
uniform float jitterFrequency;
uniform float kernelBias;
in vec2 coord;
out vec4 outputColor;

// Random number generator based on input coordinates and seed
float random(vec2 st, float seed) {
  return fract(sin(seed + dot(st.xy, vec2(12.9898, 78.233))) * 43758.5453123);
}

// Perlin-like noise function
float noise(in vec2 st, float seed) {
  vec2 i = floor(st);
  vec2 f = fract(st);
  
  float a = random(i, seed);
  float b = random(i + vec2(1.0, 0.0), seed);
  float c = random(i + vec2(0.0, 1.0), seed);
  float d = random(i + vec2(1.0, 1.0), seed);
  
  vec2 u = f * f * (3.0 - 2.0 * f);
  
  return mix(a, b, u.x) +
  (c - a) * u.y * (1.0 - u.x) +
  (d - b) * u.x * u.y;
}

void main() {
  vec2 resolution = dimensions.xy;
  vec2 uv = coord / resolution;
  
  // Define horizontal blur kernel weights
  float kernel[12];
  kernel[0] = -4.0 / 9.0;
  kernel[1] = -3.0 / 9.0;
  kernel[2] = -2.0 / 9.0;
  kernel[3] = -1.0 / 9.0;
  kernel[4] = 0.0 / 9.0;
  kernel[5] = 1.0 / 9.0;
  kernel[6] = 2.0 / 9.0;
  kernel[7] = 3.0 / 9.0;
  kernel[8] = 4.0 / 9.0;
  kernel[9] = 4.0 / 9.0;
  kernel[10] = 4.0 / 9.0;
  kernel[11] = 4.0 / 9.0;
  
  // Apply kernel bias to all weights
  for (int i = 0; i < 12; i++) {
    kernel[i] += kernelBias;
  }
  
  vec3 convolvedColor = vec3(0.0);
  
  // Calculate horizontal sync jitter based on noise with intensity and frequency control
  float hsync = floor(noise(vec2(uv.y * 50.0, 0.0), time * 5.5383 * jitterFrequency) * 3.0 * jitterIntensity);
  
  // Apply horizontal blur with jitter and intensity control
  for (int i = -4; i <= 4; i++) {
    vec2 offset = vec2(float(i), 0);
    convolvedColor += texture(tex, uv + (offset + vec2(hsync, 0.0)) / resolution).rgb * kernel[i + 6] * blurIntensity;
  }
  
  outputColor = vec4(convolvedColor, 1.0);
}
