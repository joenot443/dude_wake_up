#version 150

// Required uniforms for OpenFrameworks shader
uniform sampler2D tex;
uniform vec2 dimensions;
uniform float time;
uniform vec2 center;
uniform float size;
in vec2 coord;
out vec4 outputColor;

// Number of iterations for the swirling effect
#define ITERATION_COUNT 218.0

// Color palette function using cosine interpolation
vec3 getPaletteColor(in float t) {
  vec3 color1 = vec3(0.184, 0.776, 0.737);
  vec3 color2 = vec3(0.702, 0.702, 0.702);
  vec3 color3 = vec3(0.788, 0.188, 0.910);
  vec3 color4 = vec3(0.510, 0.510, 0.510);
  
  return color1 + color2 * cos(6.28318 * (color3 * t + color4));
}

void main() {
  // Time-based animation
  float animTime = time * 6.28 * 0.1;
  
  // Center and normalize coordinates
  vec2 centeredCoord = coord - dimensions.xy * 0.5 * center;
  vec2 normalizedCoord = centeredCoord / dimensions.xx * 15.0 * (4.0 - size);
  
  // Initialize effect parameters
  float distance = length(normalizedCoord);
  float effect = 0.0;
  float phase = animTime;
  float direction = 1.0;
  float angle = 0.0;
  float lengthFactor = -distance * (cos(animTime) * 0.2 + 0.2);
  
  // Main swirling effect loop
  for(float i = 0.0; i < ITERATION_COUNT; i += 1.0) {
    float currentPhase = phase + (sin(i + animTime) - 1.0) * 0.05 + lengthFactor;
    angle = dot(normalize(normalizedCoord), normalize(vec2(cos(currentPhase * direction), sin(currentPhase * direction))));
    angle = max(0.0, angle);
    angle = pow(angle, 20.0);
    direction *= -1.0;
    phase += mod(i, 6.28);
    
    effect += angle;
    effect = abs(mod(effect + 1.0, 2.0) - 1.0);
    effect = cos(abs(effect * 1.3));
  }
  
  // Add shine and scale effects
  float shine = 1.2;
  float scale = 0.5;
  effect += shine - distance * (scale + sin(animTime + dot(normalize(normalizedCoord), vec2(1.0, 0.0)) * 12.0) * 0.01);
  effect = max(effect, 0.0);
  
  // Final color processing
  vec3 highlightColor = vec3(0.984, 0.514, 0.984);
  centeredCoord += dimensions.xy * 0.5;
  vec2 finalCoord = centeredCoord / dimensions.xx;
  
  vec3 paletteColor = getPaletteColor(phase);
  vec3 finalColor = mix(vec3(0.0), highlightColor, effect);
  
  // Clamp and adjust final color
  finalColor = min(max(finalColor, 0.0), 1.0);
  finalColor = 1.0 - vec3(0.6, 0.4, 0.3) * 3.0 * (1.0 - finalColor);
  
  outputColor = vec4(finalColor, 1.0);
}

