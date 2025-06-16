#version 150

uniform sampler2D tex;
uniform vec2 dimensions;
uniform float time;
uniform float amount; // Input glitch amount
uniform float blockSize; // Controls the size/frequency of glitch blocks
uniform float aberrationAmount; // Controls chromatic aberration intensity
uniform float speed; // Controls the animation speed

in vec2 coord; // Input texture coordinate from vertex shader
out vec4 outputColor; // Output fragment color

// Clamp value to [0.0, 1.0]
float sat( float t ) {
  return clamp( t, 0.0, 1.0 );
}

// Clamp vector components to [0.0, 1.0]
vec2 sat( vec2 t ) {
  return clamp( t, 0.0, 1.0 );
}

// Remap value t from interval [a, b] to [0, 1]
float remap( float t, float a, float b ) {
  return sat( (t - a) / (b - a) );
}

// Linear interpolation: /\ shape (t=[0, 0.5, 1] -> y=[0, 1, 0])
float linterp( float t ) {
  return sat( 1.0 - abs( 2.0 * t - 1.0 ) );
}

// Calculate spectrum offset for chromatic aberration effect
vec3 spectrum_offset( float t ) {
  vec3 result;
  float lowerHalf = step(t, 0.5);
  float upperHalf = 1.0 - lowerHalf;
  // Remap t to get a weight for interpolation
  float weight = linterp( remap( t, 1.0 / 6.0, 5.0 / 6.0 ) );
  float inverseWeight = 1.0 - weight;
  // Combine channels based on weight
  result = vec3(lowerHalf, 1.0, upperHalf) * vec3(inverseWeight, weight, inverseWeight);
  // Apply gamma correction
  return pow( result, vec3(1.0 / 2.2) );
}

// Pseudo-random number generator [0, 1] based on 2D input
float rand( vec2 n ) {
  return fract(sin(dot(n.xy, vec2(12.9898, 78.233))) * 43758.5453);
}

// Signed pseudo-random number generator [-1, 1]
float srand( vec2 n ) {
  return rand(n) * 2.0 - 1.0;
}

// Truncate float value to a specified number of levels
float mytrunc( float x, float num_levels ) {
  return floor(x * num_levels) / num_levels;
}

// Truncate vec2 components to a specified number of levels
vec2 mytrunc( vec2 x, float num_levels ) {
  return floor(x * num_levels) / num_levels;
}

void main() {
  // Normalize texture coordinates
  vec2 uv = coord.xy / dimensions.xy;

  // Scale and wrap time for variation, now controlled by speed uniform
  float scaledTime = mod(time * speed * 100.0, 32.0) / 110.0; // Modified

  // Normalize the input glitch amount
  float normalizedGlitchAmount = sat( amount );

  // --- Random Value Generation ---

  // Time-based random value affecting overall glitch pattern (scaled by blockSize)
  float timeBasedRandom = rand( mytrunc( vec2(scaledTime, scaledTime), 6.0 * blockSize ) ); // Modified

  // Factor derived from time and glitch amount, influences horizontal effects
  float horizontalRandomFactor = sat((1.0 - normalizedGlitchAmount) * 0.7 + timeBasedRandom);

  // Random value based on horizontally truncated UV and time (scaled by blockSize)
  float horizontalRandomValue = rand( vec2(mytrunc( uv.x, 10.0 * blockSize * horizontalRandomFactor ), scaledTime) ); // Modified

  // Factor derived from horizontal randomness and glitch amount, influences vertical effects
  // Calculation avoids potential driver issues with clamp/max by ensuring value < 1.0
  float verticalRandomFactorTemp = 0.5 - 0.5 * normalizedGlitchAmount + horizontalRandomValue;
  float verticalRandomFactor = 1.0 - max( 0.0, ((verticalRandomFactorTemp < 1.0) ? verticalRandomFactorTemp : 0.9999999) );

  // Random value based on vertically truncated UV and time (scaled by blockSize)
  float verticalRandomValue = rand( vec2(mytrunc( uv.y, 40.0 * blockSize * verticalRandomFactor ), scaledTime) ); // Modified

  // Saturation of the vertical random value, used for horizontal offset intensity
  float verticalSaturation = sat( verticalRandomValue );

  // Random value for Y-offset calculation, based on horizontal factor (scaled by blockSize)
  float yOffsetRandomValue = rand( vec2(mytrunc( uv.y, 10.0 * blockSize * horizontalRandomFactor ), scaledTime) ); // Modified

  // Factor determining the Y-offset amount and direction
  float yOffsetFactor = (1.0 - sat(yOffsetRandomValue + 0.8)) - 0.1;

  // Pixel-specific random value for fine-tunin g offsets
  float pixelRandomValue = rand( uv + scaledTime );

  // --- Offset Calculation ---

  // Calculate the base horizontal offset
  float baseHorizontalOffset = 0.05 * verticalSaturation * amount * ( timeBasedRandom > 0.5 ? 1.0 : -1.0 );
  // Modulate horizontal offset with pixel-level randomness
  float horizontalOffset = baseHorizontalOffset + 0.5 * pixelRandomValue * baseHorizontalOffset;

  // Apply vertical offset
  uv.y += 0.1 * yOffsetFactor * amount;

  // --- Chromatic Aberration Sampling ---
  const int NUM_SAMPLES = 20;
  const float RCP_NUM_SAMPLES_F = 1.0 / float(NUM_SAMPLES);

  vec4 accumulatedColor = vec4(0.0);
  vec3 accumulatedWeight = vec3(0.0);

  for( int i = 0; i < NUM_SAMPLES; ++i ) {
    float sampleTime = float(i) * RCP_NUM_SAMPLES_F;
    // Apply time-varying horizontal offset for each sample,
    // scaled by aberrationAmount uniform
    vec2 sampleUv = uv;
    sampleUv.x = sat( sampleUv.x + horizontalOffset * sampleTime * aberrationAmount ); // Modified

    // Sample texture at offset UV
    vec4 sampleColor = texture( tex, sampleUv, -10.0 ); // Use mip-mapping bias

    // Calculate color spectrum weight for this sample time
    vec3 spectrumWeight = spectrum_offset( sampleTime );

    // Weight the sample color by the spectrum offset
    sampleColor.rgb = sampleColor.rgb * spectrumWeight;

    // Accumulate weighted color and total weight
    accumulatedColor += sampleColor;
    accumulatedWeight += spectrumWeight;
  }

  // Normalize accumulated color by total weight
  // Avoid division by zero if accumulatedWeight components are zero
  if (accumulatedWeight.r > 0.0) accumulatedColor.r /= accumulatedWeight.r;
  if (accumulatedWeight.g > 0.0) accumulatedColor.g /= accumulatedWeight.g;
  if (accumulatedWeight.b > 0.0) accumulatedColor.b /= accumulatedWeight.b;

  // Average the alpha component
  accumulatedColor.a *= RCP_NUM_SAMPLES_F;

  // Set the final output color
  outputColor = accumulatedColor;
}

