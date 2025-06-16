#version 150

// Uniform inputs
uniform float time;              // Time for animation
uniform float colorWarp;         // Controls color cycling intensity
uniform vec2 dimensions;         // Screen dimensions
uniform float waveFrequency;      // Controls speed of wave distortion
uniform float waveAmplitude;      // Controls intensity of wave distortion
uniform float ringDensity;        // Controls spacing between rings
uniform float ringOffset;         // Controls phase offset of rings
uniform float colorSpeed;         // Controls speed of color cycling
uniform float colorPhase;         // Controls phase offset of colors
in vec2 coord;                   // Texture coordinates
out vec4 outputColor;            // Final output color

void main(void) {
  // Base scaling and ring parameters
  float baseScale = dimensions.y / 100.0;
  float numRings = 50.0;
  float maxRadius = dimensions.x * 1.0;
  float ringGap = baseScale * 0.5;
  
  // Center the position
  vec2 centeredPos = gl_FragCoord.xy - dimensions.xy * 0.5;
  float distFromCenter = length(centeredPos);

  // Add wave distortion
  float waveDistortion = (sin(centeredPos.y * waveFrequency / baseScale + time) *
                         sin(centeredPos.x * waveFrequency / baseScale + time * 0.5)) *
                        baseScale * waveAmplitude;
  distFromCenter += waveDistortion;

  // Calculate ring pattern
  float ringPattern = mod(distFromCenter + maxRadius / (numRings * 2.0) + ringOffset, 
                         maxRadius / (numRings * ringDensity));
  ringPattern = abs(ringPattern - maxRadius / (numRings * 2.0));
  ringPattern = clamp(ringPattern - ringGap, 0.0, 1.0);

  // Generate color cycling
  float normalizedDist = distFromCenter / maxRadius;
  vec3 colorCycle = fract((normalizedDist - 1.0) * 
                         vec3(numRings * -0.5, -numRings, numRings * 0.25) * 
                         0.5 * colorWarp * colorSpeed + colorPhase);

  outputColor = vec4(colorCycle * ringPattern, 1.0);
}
