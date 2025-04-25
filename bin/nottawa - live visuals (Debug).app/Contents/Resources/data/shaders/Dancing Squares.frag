#version 150

// Removed unused uniforms: tex, audio
// Removed unused define: SIZE

uniform vec2 dimensions;
uniform float time;

in vec2 coord; // Input pixel coordinates
out vec4 outputColor; // Output fragment color

void main() {
  // Normalize coordinates, rotate, shift, and scale
  vec2 normalizedCoord = (coord - 0.5 * dimensions.xy) / dimensions.x;
  vec2 rotatedCoord = normalizedCoord * mat2(1, 1, -1, 1) / sqrt(2.0);
  vec2 shiftedCoord = rotatedCoord + 0.5;
  vec2 scaledCoord = shiftedCoord * 10.0;

  // Create a repeating grid pattern
  vec2 gridFraction = fract(scaledCoord);
  // Fold the pattern towards the center of each grid cell
  vec2 foldedFraction = min(gridFraction, 1.0 - gridFraction);

  // Calculate base pattern value within the folded cell
  float patternValue = fract(4.0 * min(foldedFraction.x, foldedFraction.y));

  // Calculate distance from the center of the 10x10 grid
  float distanceFromGridCenter = length(floor(scaledCoord) - 4.5);

  // Create a radial wave based on time and distance
  float radialTimeWave = 2.0 * fract((time - distanceFromGridCenter * 0.5) / 4.0) - 1.0; // Range [-1, 1]

  // Invert the pattern based on the wave's sign
  patternValue = (radialTimeWave < 0.0) ? patternValue : 1.0 - patternValue;

  // Apply smoothstep based on pattern and wave, adding a distance component
  float finalIntensity = smoothstep(-0.05, 0.0, 0.95 * patternValue - radialTimeWave * radialTimeWave) + distanceFromGridCenter * 0.1;

  // Mix between white and light blue based on the final intensity
  vec4 colorA = vec4(1.0); // White
  vec4 colorB = vec4(0.5, 0.75, 1.0, 1.0); // Light Blue
  outputColor = mix(colorA, colorB, finalIntensity);
}
