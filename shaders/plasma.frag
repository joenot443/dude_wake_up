#version 150

// Input uniforms
uniform sampler2DRect tex;
uniform float time;
uniform float speed;
uniform float color;
uniform float alpha;
uniform float gamma;
uniform float beta;
uniform float delta;
uniform vec2 plasma;
uniform vec2 dimensions;

// Input/output variables
in vec2 coord;
out vec4 outputColor;

void main()
{
  // Scale time by speed parameter
  float scaledTime = time / 0.1 * speed;
  
  // Calculate normalized coordinates
  vec2 currentCoord = coord;
  vec2 resolution = dimensions.xy;
  vec2 scaleFactor = vec2(160, 100);
  
  // Transform coordinates to centered space
  vec2 normalizedPos = (currentCoord + currentCoord - resolution) / resolution * scaleFactor;
  
  // Calculate dynamic offset based on time
  vec2 timeOffset = vec2(cos(-scaledTime / 165.0), cos(scaledTime / 45.0)) * scaleFactor - normalizedPos;
  
  // Calculate plasma effect intensity
  float plasmaIntensity = 1.0 + 
    cos(length(vec2(cos(scaledTime / 98.0), sin(scaledTime / 178.0)) * scaleFactor - normalizedPos) / (30.0 * gamma)) +
    cos(length(vec2(sin(-scaledTime / 124.0), cos(scaledTime / 104.0)) * scaleFactor - normalizedPos) / (20.0 * gamma)) +
    sin(length(timeOffset) / (25.0 * beta)) * sin(timeOffset.x / 20.0) * sin(timeOffset.y / (15.0 * delta));
  
  // Generate color based on time and intensity
  vec4 baseColor = alpha + 0.5 * cos(scaledTime / vec4(63.0 * (1.0 - color), 78.0, 45.0, 1.0) + 
                                  vec4(28.0, 49.0, 14.0, 0.0) * color + 
                                  (plasmaIntensity + vec4(0.0, 1.0, -0.5, 0.0)) * 3.14);
  
  // Apply alpha and output final color
  outputColor = vec4(baseColor.rgb, 1.0);
}
