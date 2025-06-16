#version 150

uniform sampler2D tex;
uniform vec2 dimensions;
uniform float time;
// New uniforms
uniform float paletteMorph;
uniform float gyroidComplexity;
uniform float surfaceDistortion;
uniform int maxSteps;
uniform float aoIntensity;
uniform vec3 fresnelColor;

in vec2 coord;
out vec4 outputColor;

// Generates a color based on a time parameter 't'
vec3 colorPalette(float t) {
  vec3 baseColor = vec3(0.45);
  vec3 colorOffset = vec3(0.35);
  // Uses cosine waves to create a smooth color transition
  return baseColor + colorOffset * cos(6.28318 * (t * vec3(1.0) + vec3(0.7, 0.39, 0.2)));
}

// Second color palette (different trigonometric function)
vec3 colorPalette2(float t) {
  vec3 baseColor = vec3(0.6, 0.5, 0.4); // Warmer base
  vec3 colorOffset = vec3(0.4, 0.3, 0.5);
  // Different phases and slightly different frequency multiplier
  return baseColor + colorOffset * cos(6.28318 * (t * vec3(0.9) + vec3(0.1, 0.9, 0.6)));
}

// Calculates the gyroid value at a given point 'p' with a specific scale
float gyroid(vec3 p, float scale) {
  p *= scale;
  // Bias calculation introduces variation based on time and position
  float bias = mix(1.1, 2.65, sin(time * 0.4 + p.x / 3.0 + p.z / 4.0) * 0.5 + 0.5);
  // Core gyroid formula: dot product of sine and cosine of scaled coordinates
  // Incorporate gyroidComplexity
  float gyroidValue = abs(dot(sin(p * 1.01 * gyroidComplexity), cos(p.zxy * 1.61 * gyroidComplexity)) - bias) / (scale * 1.5) - 0.1;
  return gyroidValue;
}

// Defines the overall scene based on the gyroid function
float scene(vec3 p) {
  // Incorporate surfaceDistortion
  vec3 distortedP = p + sin(p * 5.0) * surfaceDistortion; // Example distortion pattern frequency
  float g1 = 0.7 * gyroid(distortedP, 4.0); // Use distorted point
  return g1;
}

// Calculates the surface normal at point 'p' using finite differences
vec3 calculateNormal(vec3 p) {
  // Create slightly offset points to estimate the gradient
  mat3 k = mat3(p, p, p) - mat3(0.01);
  // Calculate the normal by comparing scene values at offset points
  return normalize(scene(p) - vec3(scene(k[0]), scene(k[1]), scene(k[2])));
}

void main() {
  // Normalize texture coordinates to range roughly from -1 to 1, centered
  vec2 uv = (coord - 0.5 * dimensions.xy) / dimensions.y;
  
  // Initial position for the ray marching
  vec3 initialPosition = vec3(time * -0.5, 1.5, 0.3);
  // Calculate the direction of the camera ray
  vec3 cameraDirection = normalize(vec3(1.0, uv));
  
  vec3 p = initialPosition;
  bool hit = false;
  // Ray marching loop: Use maxSteps uniform
  for (int i = 0; i < maxSteps && !hit; i++) {
    // Stop if the ray goes too far
    if (distance(p, initialPosition) > 8.0) break;
    // Calculate the distance to the nearest surface
    float distanceToSurface = scene(p);
    // Check if the ray is close enough to the surface to consider it a hit
    if (distanceToSurface * distanceToSurface < 0.00001) hit = true;
    // Move the point along the camera direction by the distance to the surface
    p += cameraDirection * distanceToSurface;
  }
  
  // Calculate the surface normal at the hit point
  vec3 surfaceNormal = calculateNormal(p);
  
  // Calculate ambient occlusion based on nearby scene density
  float ao = 1.0 - smoothstep(-0.3, 0.75, scene(p + surfaceNormal * 0.4))
             * smoothstep(-3.0, 3.0, scene(p + surfaceNormal * 1.0));
  // Apply AO Intensity
  float finalAO = pow(ao, aoIntensity); // Using pow for smoother control
  
  // Calculate Fresnel mix factor based on the angle between camera direction and surface normal
  float fresnelMix = pow(max(0.0, 0.8 - abs(dot(cameraDirection, surfaceNormal))), 3.0);
  
  // Apply a vignette effect based on distance from the center
  vec3 vignetteEffect = smoothstep(0.0, 1.0, vec3(1.0 - (length(uv * 0.8) - 0.1)));
  
  // Modulate palette input time based on Fresnel effect and fresnelColor.r
  float timeOffset = (fresnelColor.r - 0.5) * 2.0; // Map R [0,1] to [-1, 1]
  float modulatedPaletteTime = 0.1 - time * 0.01 + p.x * 0.28 + p.y * 0.2 + p.z * 0.2 + fresnelMix * timeOffset * 0.5; // Adjust offset strength

  // Get colors from palettes using modulated time
  vec3 color1 = colorPalette(modulatedPaletteTime);
  vec3 color2 = colorPalette2(modulatedPaletteTime); 
  // Interpolate between palettes using paletteMorph
  vec3 baseColor = mix(color1, color2, paletteMorph);
  
  // Modulate final baseColor brightness/contrast based on fresnelColor.g and fresnelMix
  float brightnessFactor = 1.0 + (fresnelColor.g - 0.5) * fresnelMix; // Use G to adjust brightness +/- 50% at edges
  baseColor *= brightnessFactor;

  // Apply ambient occlusion to the result
  vec3 shadedColor = baseColor * finalAO; // No direct fresnelColor mixing here anymore
  
  // If no surface was hit, or the hit point is far away, fade to black
  vec3 finalColor = mix(shadedColor, vec3(0.0), !hit ? 1.0 : smoothstep(0.0, 8.0, distance(p, initialPosition)));
  // Apply vignette effect
  finalColor = mix(vec3(0.0), finalColor, vignetteEffect + 0.1);
  // Apply a final smoothstep for contrast, modulated by time and position
  finalColor = smoothstep(0.0, 1.0 + 0.3 * sin(time + p.x * 4.0 + p.z * 4.0), finalColor);
  
  // Set the final output color
  outputColor.xyz = finalColor;
  // Apply gamma correction (sqrt)
  outputColor.xyz = sqrt(outputColor.xyz);
  outputColor.w = 1.0;
}
