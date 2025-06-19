#version 150

uniform sampler2D tex;
uniform float audio[256];
uniform vec2 dimensions;
uniform float time;
in vec2 coord;
out vec4 outputColor;

// Returns 3 B-spline functions of degree 2
vec3 B2_spline(vec3 x) {
  vec3 t = 3.0 * x;
  vec3 b0 = step(0.0, t) * step(0.0, 1.0 - t);
  vec3 b1 = step(0.0, t - 1.0) * step(0.0, 2.0 - t);
  vec3 b2 = step(0.0, t - 2.0) * step(0.0, 3.0 - t);
  return 0.5 * (b0 * pow(t, vec3(2.0)) +
                b1 * (-2.0 * pow(t, vec3(2.0)) + 6.0 * t - 3.0) +
                b2 * pow(3.0 - t, vec3(2.0)));
}

void main() {
  // Normalized pixel coordinates (0 to 1)
  vec2 uv = coord.xy / dimensions.xy;

  // Vertical bars setup
  float verticalBarCount = 256.0;
  float horizontalSpacing = 1.0;
  float horizontalFrequency = uv.x * 3.14159; // Use PI constant
  float horizontalSquareWave = sign(sin(horizontalFrequency * verticalBarCount) + 1.0 - horizontalSpacing);

  // Get audio sample for current horizontal position
  float audioSample = audio[int(uv.x * 256.0)];

  // Calculate FFT visualization factor based on audio and square wave
  float fftFactor = horizontalSquareWave * audioSample * 0.5;

  // Horizontal bars setup
  float horizontalBarCount = 100.0;
  float verticalSpacing = 0.180;
  float verticalFrequency = uv.y * 3.14159; // Use PI constant
  float verticalSquareWave = sign(sin(verticalFrequency * horizontalBarCount) + 1.0 - verticalSpacing);

  // Centered UV coordinates (-1 to 1)
  vec2 centeredUV = uv * 2.0 - 1.0; // Corrected centering

  // Time scaled for spline calculation
  float timeScaled = time / 100.0;
  float polychromeFactor = 1.0;
  vec3 splineArgs =
      fract(vec3(polychromeFactor * uv.x - timeScaled) + vec3(0.0, -1.0 / 3.0, -2.0 / 3.0));
  vec3 splineColorBase = B2_spline(splineArgs);

  // Flame color calculation based on spline and vertical position
  float verticalMagnitude = abs(centeredUV.y);
  vec3 baseColor = vec3(1.0) - verticalMagnitude * splineColorBase;
  vec3 flameColor = pow(baseColor, vec3(3.0));

  // Calculate vertical position difference and direction factor
  float verticalDifference = 0.3 - uv.y;
  float directionFactor = sign(verticalDifference);
  directionFactor = (directionFactor + 1.0) * 0.5; // Map -1/1 to 0/1

  // Combine flame color, FFT factor, and vertical bars
  vec3 finalColor = flameColor * vec3(1.0 - step(fftFactor, abs(0.3 - uv.y))) * vec3(verticalSquareWave);

  // Apply dimming based on direction
  finalColor -= finalColor * directionFactor * 0.180;

  // Output final color
  outputColor = vec4(finalColor, 1.0);
}
