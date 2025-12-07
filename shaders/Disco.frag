#version 150

// --- Macros (Unchanged) ---
// Removed defines, replaced with uniforms below
// #define SCALE_X 24.0 // Horizontal scaling factor for grid cells
// #define SCALE_Y 10.0  // Vertical scaling factor for grid cells
// #define TIME_SCALE 0.1 // Speed factor for time-based animations
// #define BORDER 9.5   // Threshold for cell animation state change

// --- Uniforms (Unchanged) ---
uniform sampler2D tex;       // Texture sampler (unused in this specific code)
uniform vec2 dimensions; // Dimensions of the output (e.g., screen width/height)
uniform float time;        // Global time uniform
uniform float scaleX;     // Horizontal scaling factor for grid cells (formerly SCALE_X)
uniform float scaleY;     // Vertical scaling factor for grid cells (formerly SCALE_Y)
uniform float timeScale;  // Speed factor for time-based animations (formerly TIME_SCALE)
uniform float border;     // Threshold for cell animation state change (formerly BORDER)

// --- Inputs/Outputs (Unchanged) ---
in vec2 coord;         // Input fragment coordinates (pixel coordinates)
out vec4 outputColor;  // Final output color for the fragment

// --- Helper Functions ---

/**
 * @brief Generates a pseudo-random float between 0.1 and 1.1 based on input.
 * @param inputVal The input float value.
 * @return A pseudo-random float.
 */
float hash1( float inputVal ) {
  // Simple hash function using sine and large number multiplication
  return fract(sin(inputVal) * 437518.56453) + 0.1;
}

/**
 * @brief Creates a smoothed box shape within a 0.0 to 1.0 coordinate space.
 * @param coordInBox The 2D coordinates within the unit box.
 * @return A float representing the box shape intensity (higher inside, lower outside/near edge).
 */
float box(vec2 coordInBox) {
  vec2 borderSize = vec2(0.001); // Size of the border area
  vec2 borderSmoothness = vec2(0.05); // Width of the smooth transition at the border
  
  // Create smooth transitions from edge to borderSize
  vec2 smoothCoord = smoothstep(borderSize, borderSize + borderSmoothness, coordInBox);
  // Create smooth transitions from (1-borderSize) to the inner edge
  smoothCoord *= smoothstep(borderSize, borderSize + borderSmoothness, vec2(1.0) - coordInBox);
  
  // Combine transitions and slightly dim the center
  return smoothCoord.x * smoothCoord.y * (1.0 - length(0.5 - coordInBox) * 0.75);
}

/**
 * @brief Converts a color from Hue-Saturation-Brightness (HSB) space to Red-Green-Blue (RGB) space.
 * @param hsbColor The input color in HSB format (x=H, y=S, z=B). Hue is expected in [0, inf).
 * @return The equivalent color in RGB format.
 */
vec3 hsb2rgb( in vec3 hsbColor ){
  // Standard HSB to RGB conversion formula
  vec3 rgb = clamp(abs(mod(hsbColor.x * 6.0 + vec3(0.0, 4.0, 2.0), 6.0) - 3.0) - 1.0, 0.0, 1.0 );
  rgb = rgb * rgb * (3.0 - 2.0 * rgb); // Smoothing step
  return hsbColor.z * mix(vec3(1.0), rgb, hsbColor.y); // Mix white with calculated hue/saturation based on brightness
}

/**
 * @brief Generates a brightness value based on position and time (simplified noise/FBM).
 * @param gridPosition The integer grid cell coordinates (column, row).
 * @return A brightness value, typically between 0.75 and 1.0.
 */
float fbm (in vec2 gridPosition) {
  // Calculate a time-varying value based on hashed grid position
  float value = (sin(time * 5.0 * hash1(gridPosition.x + hash1(gridPosition.y))) + 4.0) * 0.25;
  // Ensure a minimum brightness and apply power curve
  return max(0.75, pow(value, 3.0));
}


// --- Main Shader Program ---
void main()
{
  // --- Coordinate Transformation ---
  
  // 1. Normalize coordinates: Map pixel coords (coord) to a system where
  //    the center is (0,0) and the vertical extent is roughly -1 to 1.
  vec2 screenSize = dimensions.xy;
  vec2 normalizedCoord = (coord * 2.0 - screenSize) / screenSize.y;
  
  // 2. Convert to Polar-like coordinates: Calculate angle and a modified radius (1/radius).
  //    This transforms the space into radial bands.
  float angle = atan(normalizedCoord.x, normalizedCoord.y); // Angle component
  float radius = length(normalizedCoord);                   // Distance from center
  vec2 polarInvRadiusCoord = vec2(angle, 1.0 / radius);      // Use 1/radius for vertical mapping
  
  // --- Cell Calculation Setup ---
  vec3 finalColor = vec3(0.0); // Initialize final color to black
  float cellMask = 1.0;        // Factor controlling cell brightness/visibility

  // scaleY now ranges from 0.0 to 2.0, so multiply by 50 to get effective 0-100 range
  float effectiveScaleY = scaleY * 50.0;

  // Calculate a local time cycle based on vertical scaling
  // This value loops from 0 to 1/scaleY
  float localTimeCycle = mod(time * timeScale, 1.0 / effectiveScaleY);
  
  // --- Map Polar Coordinates to Grid ---

  // Scale angle by horizontal factor and wrap around circle
  // scaleX now ranges from 0.0 to 1.0, so multiply by 100 to get effective 0-100 range
  polarInvRadiusCoord.x *= (scaleX * 100.0) / (2.0 * 3.1415926); // Scale angle to fit scaleX columns
  float columnIndex = floor(polarInvRadiusCoord.x);      // Integer part = column index
  polarInvRadiusCoord.x = fract(polarInvRadiusCoord.x); // Fractional part = horizontal position within cell
  
  // --- Determine Cell State and Calculate Color ---
  
  // Check if the fragment falls into the "active" region based on its inverse radius and the time cycle
  if (polarInvRadiusCoord.y < border - localTimeCycle)
  {
    // State 1: "Active" or recently spawned cells
    
    // Animate vertical position based on global time
    polarInvRadiusCoord.y += time * timeScale;
    // Scale vertical position by vertical factor
    polarInvRadiusCoord.y *= effectiveScaleY;
    
    float rowIndex = floor(polarInvRadiusCoord.y); // Integer part = row index
    polarInvRadiusCoord.y = fract(polarInvRadiusCoord.y); // Fractional part = vertical position within cell
    
    // Calculate cell color:
    // - Hue derived from hashed row/column indices
    // - Saturation fixed at 0.7
    // - Brightness from fbm function based on indices
    // Apply the box shape to the color
    vec2 cellGridIndex = vec2(columnIndex, rowIndex);
    vec3 cellHSBColor = vec3(hash1(rowIndex + hash1(columnIndex)), 0.7, fbm(cellGridIndex));
    finalColor = hsb2rgb(cellHSBColor) * box(polarInvRadiusCoord);
  }
  else {
    // State 2: "Fading" or "trailing" cells
    
    // Calculate fade factor based on inverse radius (cells further out fade more)
    float fadeFactor = smoothstep(3.0, 0.0, sqrt(polarInvRadiusCoord.y));
    
    // Adjust vertical position based on where it is relative to the BORDER boundary
    polarInvRadiusCoord.y += localTimeCycle - border;
    
    // Calculate row index considering the time cycle and border offset
    float effectiveTime = (border - localTimeCycle + time * timeScale);
    float rowIndex = floor(effectiveTime * effectiveScaleY + floor(polarInvRadiusCoord.y));
    vec2 cellGridIndex = vec2(columnIndex, rowIndex);
    
    // Complex calculation for vertical position within the fading cell, creating a trail effect
    // Includes scaling and an offset based on time, scale factors, and hash randomness.
    float randomTrailSpeedFactor = hash1(columnIndex + hash1(rowIndex));
    float trailOffset = min(0.0, 1.0 - effectiveScaleY + localTimeCycle * (effectiveScaleY / timeScale * effectiveScaleY * randomTrailSpeedFactor + 1.0));
    polarInvRadiusCoord.y = fract(polarInvRadiusCoord.y) * effectiveScaleY + trailOffset;
    
    
    // Start with the box shape for the cell mask
    cellMask *= box(polarInvRadiusCoord);
    
    // Add a trailing effect intensity based on position within the cell
    float trailIntensity = smoothstep(3.0, 1.0, sqrt(polarInvRadiusCoord.y)); // Fade based on overall radius
    trailIntensity *= smoothstep(1.0, 1.05, polarInvRadiusCoord.y);           // Intensity near top edge
    trailIntensity *= smoothstep(0.5, 0.45, abs(polarInvRadiusCoord.x - 0.5)); // Intensity near horizontal center
    cellMask += trailIntensity * 0.25; // Add trail brightness
    
    // Apply the overall fade factor
    cellMask *= fadeFactor;
    
    // Calculate the base cell color (same method as state 1)
    vec3 cellHSBColor = vec3(hash1(rowIndex + hash1(columnIndex)), 0.7, fbm(cellGridIndex));
    vec3 baseCellColor = hsb2rgb(cellHSBColor);
    
    // Mix the base cell color with the background (black) based on the final cell mask
    finalColor = mix(vec3(0.0), baseCellColor, cellMask);
  }
  
  // --- Final Output ---
  outputColor = vec4(finalColor, 1.0); // Set alpha to 1.0 (fully opaque)
}
