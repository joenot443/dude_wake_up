#version 150

// Required uniforms for OpenFrameworks
uniform sampler2D tex;
uniform vec2 dimensions;
uniform float time;
uniform float height;

// New control parameters
uniform float lineDensity;    // Controls spacing between lines
uniform float noiseIntensity; // Controls the amount of noise in the waveform
uniform float waveFrequency;  // Controls the frequency of the base wave
uniform float timeScale;      // Controls the speed of the animation

in vec2 coord;
out vec4 outputColor;

// Integer hash function for generating pseudo-random values
float hash(uint n) {
  n = (n << 13U) ^ n;
  n = n * (n * n * 15731U + 789221U) + 1376312589U;
  return float(n & uvec3(0x0fffffffU)) / float(0x0fffffff);
}

// Basic noise function for smooth interpolation
float basicNoise(float x) {
  float i = floor(x);
  float f = fract(x);
  float k = fract(i * 0.1731);
  return f * (f - 1.0) * ((18.0 * k - 4.0) * f * (f - 1.0) - 1.0);
}

// Calculate distance field for the waveform visualization
float calculateDistance(vec2 uv) {
  float lineWidth = 0.007 / lineDensity;  // Adjust line width based on density
  uvec2 bounds = uvec2((0.27 / lineWidth), (0.71 / lineWidth));  // Min/max line bounds
  uint lineId = uint(uv.y / lineWidth);  // Current line index
  
  // Check nearby lines for intersection
  for (uint i = max(bounds.x, lineId - 10u); i <= min(lineId, bounds.y); ++i) {
    float randomValue = hash(i);
    float timeOffset = 2.0 * randomValue * time * timeScale + randomValue;
    float xPos = uv.x * 10.0;
    
    // Calculate noise-based offsets with intensity control
    float baseNoise = basicNoise(xPos * 1.5 + timeOffset) * noiseIntensity;
    float detailNoise = basicNoise(xPos * 8.0 + 4.0 * timeOffset) * noiseIntensity;
    float waveFactor = max(sin(xPos * waveFrequency + 0.3), 0.0) * height;
    
    // Combine noise and wave factors for final offset
    float offset = lineWidth + waveFactor * 0.06 * randomValue * (waveFactor - baseNoise - 0.2 * detailNoise) + 0.002 * detailNoise;
    float distance = uv.y - (float(i) * lineWidth) - offset;
    
    if (distance < 0.001) return distance;
  }
  return 1.0;
}

void main() {
  // Normalize coordinates to [-1, 1] range and flip y-axis
  vec2 uv = (coord - 0.5 * dimensions.xy) / min(dimensions.y, dimensions.x);
  uv.y = -uv.y;  // Flip y-axis
  uv = (uv + 1.0) / 2.0;
  
  // Calculate visibility based on x-position and distance field
  float isVisible = float(uv.x > 0.3 && uv.x < 0.7);
  isVisible *= float(abs(calculateDistance(uv)) <= 0.0008);
  
  // Output final color
  outputColor = vec4(mix(0.1, 0.9, isVisible));
}
