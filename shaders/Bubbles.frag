#version 150

// Input uniforms
uniform sampler2D tex;           // Input texture
uniform vec2 dimensions;         // Screen dimensions
uniform float time;             // Global time

// Core parameters
uniform float maxIterations;    // Number of bubble layers
uniform float nearPlane;        // Near plane distance
uniform float fadeIntensity;    // How quickly bubbles fade
uniform float bubbleOpacity;    // Overall bubble transparency

// Effect parameters
uniform float bubbleSize;       // Base size of bubbles
uniform float bubbleSpeed;      // Animation speed
uniform float colorIntensity;   // Color saturation
uniform float lightIntensity;   // Lighting effect strength

// Input/output variables
in vec2 coord;                  // Texture coordinates
out vec4 outputColor;           // Final output color

// Constants
const float PI = 3.141592654;
const float TAU = 2.0 * PI;
// Background colors
const vec3 BG_COLOR_0 = vec3(0.1, 0.0, 1.0) * 0.1;  // Dark blue
const vec3 BG_COLOR_1 = vec3(0.0, 0.4, 1.0) * 0.6;  // Light blue
const vec3 LIGHT_DIR = normalize(vec3(1.0, 1.5, 2.0));


// Hash functions for noise generation
float hash(float x) {
  return fract(sin(x * 12.9898) * 13758.5453);
}

float hash(vec2 p) {
  return fract(sin(dot(p.xy, vec2(12.9898, 58.233))) * 13758.5453);
}

// Grid modulo operation
vec2 mod2(inout vec2 p, vec2 size) {
  vec2 c = floor((p + size * 0.5) / size);
  p = mod(p + size * 0.5, size) - size * 0.5;
  return c;
}

// Hyperbolic tangent approximation
float tanh_approx(float x) {
  float x2 = x * x;
  return clamp(x * (27.0 + x2) / (27.0 + 9.0 * x2), -1.0, 1.0);
}

// Render a single bubble plane
vec4 plane(vec2 p, float index, float zFactor, float z, vec3 bgColor) {
  // Calculate bubble size and position
  float size = 0.5 * zFactor;
  vec2 bubblePos = p;
  vec2 cellIndex = mod2(bubblePos, vec2(2.0 * size, size));
  
  // Generate random values for this bubble
  float h0 = hash(cellIndex + index + 123.4);
  float h1 = fract(4483.0 * h0);  // Color variation
  float h2 = fract(8677.0 * h0);  // Time offset
  float h3 = fract(9677.0 * h0);  // X position variation
  float h4 = fract(7877.0 * h0);  // Bubble visibility
  float h5 = fract(9967.0 * h0);  // Y position variation
  
  // Early exit for invisible bubbles
  if (h4 < 0.5) {
    return vec4(0.0);
  }
  
  // Calculate bubble parameters
  float fade = exp(-fadeIntensity * max(z - 2.0, 0.0));
  float antiAlias = mix(0.0125, 2.0 / dimensions.y, fade);
  float radius = size * mix(0.1, bubbleSize, h0 * h0);
  float amplitude = mix(0.5, 0.5, h3) * radius;
  
  // Animate bubble position
  bubblePos.x -= amplitude * sin(mix(3.0, bubbleSpeed, h0) * time + TAU * h2);
  bubblePos.x += 0.95 * (size - radius - amplitude) * sign(h3 - 0.5) * h3;
  bubblePos.y += bubbleSize * (size - 2.0 * radius) * sign(h5 - 0.5) * h5;
  
  // Calculate bubble shape
  float dist = length(bubblePos) - radius;
  if (dist > antiAlias) {
    return vec4(0.0);
  }
  
  // Calculate bubble colors
  vec3 outerColor = (0.5 + 0.5 * sin(vec3(0.0, 1.0, 2.0) + h1 * TAU));
  vec3 innerColor = sqrt(outerColor);
  outerColor *= colorIntensity;
  innerColor *= lightIntensity;
  
  // Calculate bubble shading
  vec3 color = outerColor;
  float alpha = smoothstep(antiAlias, -antiAlias, dist);
  float z2 = (radius * radius - dot(bubblePos, bubblePos));
  
  if (z2 > 0.0) {
    float height = sqrt(z2);
    alpha *= mix(1.0, 0.8, height / radius);
    
    // Calculate lighting
    vec3 pos = vec3(bubblePos, height);
    vec3 normal = normalize(pos);
    float diffuse = max(dot(LIGHT_DIR, normal), 0.0);
    
    color = mix(outerColor, innerColor, diffuse * diffuse * diffuse);
  }
  
  // Apply final color adjustments
  color *= mix(0.8, 1.0, h0);
  color = mix(bgColor, color, fade);
  
  return vec4(color, alpha * bubbleOpacity);
}

// Main effect rendering
vec3 effect(vec2 p, vec2 pp) {
  // Calculate background gradient
  vec3 bgColor = mix(
    BG_COLOR_1,
    BG_COLOR_0,
    tanh_approx(1.5 * length(p))
  );
  vec3 color = bgColor;
  
  // Render bubble layers
  for (float i = 0.0; i < maxIterations; ++i) {
    float z = maxIterations - i;
    float zFactor = nearPlane / (nearPlane + maxIterations - i);
    
    // Calculate bubble plane position
    vec2 planePos = p;
    float randOffset = hash(i + 1234.5);
    planePos.y += -mix(0.2, 0.3, randOffset * randOffset) * time * zFactor;
    planePos += randOffset;
    
    // Blend bubble plane with existing color
    vec4 planeColor = plane(planePos, i, zFactor, z, bgColor);
    color = mix(color, planeColor.xyz, planeColor.w);
  }
  
  // Apply final effects
  color *= smoothstep(1.5, 0.5, length(pp));
  color = clamp(color, 0.0, 1.0);
  color = sqrt(color);
  
  return color;
}

void main() {
  vec2 uv = coord / dimensions.xy;
  uv.y = 1.0 - uv.y;  // Flip Y coordinate
  vec2 p = -1.0 + 2.0 * uv;
  vec2 pp = p;
  p.x *= dimensions.x / dimensions.y;
  
  vec3 color = effect(p, pp);
  outputColor = vec4(color, 1.0);
}