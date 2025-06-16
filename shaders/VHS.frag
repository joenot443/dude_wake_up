#version 150

uniform sampler2D tex;
uniform vec2 dimensions;
uniform float time;
uniform float chromaEmphasis;
uniform float noiseIntensity;
uniform float noiseFrequency;
uniform float crossColorIntensity;
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
  
  // Sample original color
  vec3 originalColor = texture(tex, uv).rgb;
  
  // Convert RGB to YCbCr color space
  float luminance = 0.299 * originalColor.r + 0.587 * originalColor.g + 0.114 * originalColor.b;
  float chromaBlue = 0.564 * (originalColor.b - luminance);
  float chromaRed = 0.713 * (originalColor.r - luminance);
  
  // Calculate horizontal color difference for cross-color effect
  vec3 horizontalDiff = texture(tex, uv + vec2(1.0/resolution.x, 0)).rgb - originalColor;
  float luminanceDiff = 0.299 * horizontalDiff.r + 0.587 * horizontalDiff.g + 0.114 * horizontalDiff.b;
  
  // Add noise to chroma channels with different frequencies and amplitudes
  float chromaNoise1 = noise(vec2(uv.r * 50.0, uv.g * 50.0), time * 5.5383 * noiseFrequency) * 0.1 - 0.05;
  float chromaNoise2 = noise(vec2(uv.r * 50.0, uv.g * 50.0), time * 2.2546 * noiseFrequency) * 0.1 - 0.05;
  float chromaNoise3 = noise(vec2(uv.r * 5.0, uv.g * 30.0), time * 6.4623 * noiseFrequency) * 0.15 - 0.075;
  float chromaNoise4 = noise(vec2(uv.r * 5.0, uv.g * 30.0), time * 8.4352 * noiseFrequency) * 0.15 - 0.075;
  
  // Apply noise to chroma channels with intensity control
  chromaRed += (chromaNoise1 + chromaNoise3) * noiseIntensity;
  chromaBlue += (chromaNoise2 + chromaNoise4) * noiseIntensity;
  
  // Apply emphasis to chroma channels and reduce luminance
  chromaBlue *= 1.5 * chromaEmphasis;
  chromaRed *= 1.5 * chromaEmphasis;
  luminance *= 0.75;
  
  // Apply cross-color effect with intensity control
  chromaBlue += cos(coord.x) * luminanceDiff * crossColorIntensity;
  chromaRed += sin(coord.x) * luminanceDiff * crossColorIntensity;
  
  // Convert back to RGB
  float red = luminance + 1.403 * chromaRed;
  float green = luminance - 0.344 * chromaBlue - 0.714 * chromaRed;
  float blue = luminance + 1.773 * chromaBlue;
  
  outputColor = vec4(red, green, blue, 1.0);
}

