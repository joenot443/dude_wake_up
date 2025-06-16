#version 150

uniform sampler2D tex;
uniform vec2 dimensions;
uniform float time;

uniform vec3 color;
uniform float zoom;
uniform float perspective;
uniform float gridSkew;   // New uniform
uniform float lineWobble; // New uniform
uniform float fogPosition;// New uniform

in vec2 coord;
out vec4 outputColor;

// Calculates the grid pattern intensity based on UV coordinates and a modifier
float grid(vec2 uv, float intensityModifier, float time) // Added time parameter
{
  // Determine line thickness based on y-coordinate
  vec2 lineThickness = vec2(uv.y, uv.y * uv.y * 0.2) * 0.01;

  // Animate grid scroll based on time and modifier
  uv += vec2(0.0, time * 4.0 * (intensityModifier + 0.05));
  
  // Apply line wobble effect - MORE INTERESTING VERSION!
  if (lineWobble > 0.0) { // Only compute if wobble is active
    float baseWobbleFreq = 15.0;
    float baseWobbleSpeed = 3.0;

    // Modulate frequency and speed slightly based on position for variation
    // Frequency changes horizontally, Speed changes vertically
    float freq1 = baseWobbleFreq + sin(uv.x * 1.5 + time * 0.2) * 7.0; 
    float speed1 = baseWobbleSpeed + cos(uv.y * 0.3 + time * 0.1) * 1.5; 

    // Primary wobble wave
    float wobble1 = sin(uv.y * freq1 + time * speed1);

    // Secondary, faster, smaller amplitude wobble wave, also position-dependent
    float freq2 = 25.0 + cos(uv.y * 0.8 + time * 0.3) * 8.0;
    float speed2 = 5.0 + sin(uv.x * 0.5 + time * 0.4) * 2.0;
    float wobble2 = cos(uv.y * freq2 + time * speed2 + uv.x * 3.0) * 0.4; // Added uv.x dependency

    // Combine wobbles and scale by the uniform parameter
    float combinedWobble = (wobble1 + wobble2) * lineWobble * 0.5; // Scale down slightly

    // Apply the wobble to the horizontal coordinate
    uv.x += combinedWobble;

    // Optional: Apply a subtle, related wobble to the vertical coordinate
    // uv.y += cos(uv.x * freq1 * 0.7 + time * speed1 * 0.9) * lineWobble * 0.1; 
  }

  // Create repeating grid pattern
  uv = abs(fract(uv) - 0.5);

  // Calculate base grid lines
  vec2 gridLines = smoothstep(lineThickness, vec2(0.0), uv);

  // Add thicker secondary lines influenced by the modifier
  gridLines += smoothstep(lineThickness * 5.0, vec2(0.0), uv) * 0.4 * intensityModifier;

  return clamp(gridLines.x + gridLines.y, 0.0, 3.0);
}

void main()
{
  vec2 perspectiveCoord = coord;
  // Apply perspective distortion based on uniform
  perspectiveCoord.y *= perspective;

  // Normalize coordinates and center them
  vec2 normalizedCoord = (2.0 * perspectiveCoord.xy - dimensions.xy) / dimensions.y;

  // Placeholder - currently unused, set to 1.0
  float battery = 1.0;

  // Calculate fog based on vertical position using fogPosition uniform
  float fogFactor = smoothstep(0.1, -0.02, abs(normalizedCoord.y + fogPosition)); // Use fogPosition

  // Base color for the scene
  vec3 pixelColor = vec3(0.0, 0.1, 0.2);

  // --- Start Grid Calculation ---
  vec2 gridUV = normalizedCoord;

  // Apply zoom and perspective offset for grid
  gridUV.y = 5.0 / (abs(normalizedCoord.y + fogPosition) + zoom); // Using fogPosition here too, for consistency? Maybe should revert to +0.2? Or make another param? Let's use fogPosition for now.
  gridUV.x *= gridUV.y * gridSkew; // Scale x based on the new y and gridSkew

  // Calculate grid intensity using the transformed UVs, passing time
  float gridIntensity = grid(gridUV, battery, time); // Pass time to grid function

  // Mix base color with uniform color based on grid intensity
  pixelColor = mix(pixelColor, color.xyz, gridIntensity);
  // --- End Grid Calculation ---

  // Add fog effect
  pixelColor += fogFactor * fogFactor * fogFactor;

  // Desaturate based on the battery value (currently minimal effect)
  pixelColor = mix(vec3(pixelColor.r) * 0.5, pixelColor, battery * 0.7);

  // Final output color
  outputColor = vec4(pixelColor, 1.0);
}

