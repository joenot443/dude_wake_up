#version 150

// Relative size of the halftone dot
#define DOTSIZE 1.48
// Converts degrees to radians
#define D2R(d) radians(d)
// Minimum scale/size for halftone pattern
#define MIN_S 0.0
// Maximum scale/size for halftone pattern
#define MAX_S 19.0

// Center of the smoothstep transition range
#define SMOOTHSTEP_CENTER 0.888
// Half the width of the smoothstep transition range
#define SMOOTHSTEP_HALF_WIDTH 0.288

uniform sampler2D tex;
uniform vec2 dimensions;
uniform float time;
uniform float amount; // Unused in this shader
uniform float speed;
uniform float rotate;
uniform float radius;

in vec2 coord; // Fragment coordinates in pixels
out vec4 outputColor;

// Center of the screen/texture in pixel coordinates
#define ORIGIN (0.5 * dimensions.xy)

// Converts RGB color to a normalized color and intensity representation.
// The alpha component of the output stores the intensity (max(R,G,B)).
vec4 rgbToNormalizedColorIntensity(in vec3 color)
{
  float intensity = max(max(color.r, color.g), color.b);
  // Avoid division by zero if intensity is zero
  if (intensity == 0.0) {
    return vec4(0.0, 0.0, 0.0, 0.0);
  }
  return min(vec4(color.rgb / intensity, intensity), 1.0);
}

// Converts a normalized color and intensity (stored in alpha) back to RGB.
vec3 normalizedColorIntensityToRgb(in vec4 normalizedColorIntensity)
{
  return normalizedColorIntensity.rgb * normalizedColorIntensity.a;
}

// Converts pixel coordinates to UV coordinates (0.0-1.0 range).
vec2 pixelToUvCoordinates(in vec2 pixelCoords)
{
  return pixelCoords / dimensions.xy;
}

// Snaps pixel coordinates to a grid defined by 'gridSize'.
vec2 snapToGrid(in vec2 pixelCoords, float gridSize)
{
  return pixelCoords - mod(pixelCoords, gridSize);
}

// Applies a smoothstep function to the input value.
vec4 applySmoothstep(in vec4 value)
{
  float lowerEdge = SMOOTHSTEP_CENTER - SMOOTHSTEP_HALF_WIDTH;
  float upperEdge = SMOOTHSTEP_CENTER + SMOOTHSTEP_HALF_WIDTH;
  return smoothstep(lowerEdge, upperEdge, value);
}

// Calculates a halftone dot pattern value for a given fragment.
// 'fragCoordCentered' is the fragment's coordinate relative to the screen center.
// 'rotationMat' is the rotation matrix for this specific color channel.
// 'currentScale' is the dynamic size of the grid cells.
vec4 calculateHalftonePattern(in vec2 fragCoordCentered, in mat2 rotationMat, float currentScale)
{
  // Transform fragment coordinate to rotated grid space, find cell center,
  // then apply another rotation. The second rotation is part of the original effect's specific math.
  vec2 rotatedFc = rotationMat * fragCoordCentered;
  vec2 gridCellCorner = snapToGrid(rotatedFc, currentScale);
  vec2 cellCenterInRotatedSpace = gridCellCorner + 0.5 * currentScale;
  vec2 transformedCellSamplePoint = cellCenterInRotatedSpace * rotationMat;

  // Calculate normalized distance from fragment to the transformed cell sample point.
  // This forms the basic shape of the dot (0 at center, 1 at edge).
  float dotShapeFactor = min(length(fragCoordCentered - transformedCellSamplePoint) / (DOTSIZE * 0.5 * currentScale), 1.0);

  // Sample the texture at the screen-space position corresponding to the transformed cell sample point.
  vec3 textureColorAtCell = texture(tex, pixelToUvCoordinates(transformedCellSamplePoint + ORIGIN)).rgb;
  
  // Convert sampled texture color to normalized color and intensity.
  vec4 colorIntensityAtCell = rgbToNormalizedColorIntensity(textureColorAtCell);
  
  // Add the dot shape factor to each component. This will be used by smoothstep
  // to create the visual dot.
  return colorIntensityAtCell + dotShapeFactor;
}

// Creates a 2D rotation matrix.
mat2 rotationMatrix2D(in float angleRadians)
{
  float cosAngle = cos(angleRadians);
  float sinAngle = sin(angleRadians);
  return mat2(
    cosAngle, -sinAngle,
    sinAngle, cosAngle
  );
}

void main()
{
  float currentScale;
  float currentRotationAngle;

  // Determine current scale and rotation based on speed and time, or other parameters
  if (speed < 0.1) {
    // This calculation for S depends on dimensions.x.
    // If dimensions.x is large, `abs(0.5 - ORIGIN.x)` is approx `0.5 * dimensions.x`.
    // Then `2.0 * abs(0.5 - ORIGIN.x) / dimensions.x` is approx `1.0`.
    currentScale = radius + MIN_S + (MAX_S - MIN_S) * 2.0 * abs(0.5 - ORIGIN.x) / dimensions.x;
    
    // This calculation for R has a component that evaluates to a constant:
    // `(0.5 * ORIGIN.y) / dimensions.y` simplifies to `0.25`.
    // So, effectively `D2R(180.0 * 0.25)` or `D2R(45.0)`.
    currentRotationAngle = D2R(180.0 * (0.5 * ORIGIN.y) / dimensions.y) * rotate;
  } else {
    currentRotationAngle = speed * 0.333 * time;
    currentScale = MIN_S + (MAX_S - MIN_S) * (0.5 - 0.5 * cos(time * speed));
  }
  
  vec2 fragCoordCentered = coord.xy - ORIGIN;
  
  // Define rotation matrices for CMYK channels with different angle offsets.
  mat2 cyanRotationMatrix    = rotationMatrix2D(currentRotationAngle + D2R(15.0));
  mat2 magentaRotationMatrix = rotationMatrix2D(currentRotationAngle + D2R(75.0));
  mat2 yellowRotationMatrix  = rotationMatrix2D(currentRotationAngle);
  mat2 keyRotationMatrix     = rotationMatrix2D(currentRotationAngle + D2R(45.0));
  
  // Calculate halftone patterns for each channel, apply smoothstep,
  // then convert back to RGB.
  // The .r, .g, .b, .a components from calculateHalftonePattern are used for
  // cyan, magenta, yellow, and key channels respectively.
  vec4 halftoneInputs = vec4(
    calculateHalftonePattern(fragCoordCentered, cyanRotationMatrix, currentScale).r,    // Cyan component
    calculateHalftonePattern(fragCoordCentered, magentaRotationMatrix, currentScale).g, // Magenta component
    calculateHalftonePattern(fragCoordCentered, yellowRotationMatrix, currentScale).b,  // Yellow component
    calculateHalftonePattern(fragCoordCentered, keyRotationMatrix, currentScale).a     // Key (intensity) component
  );
  
  vec4 smoothedHalftonePatterns = applySmoothstep(halftoneInputs);
  vec3 outputRgb = normalizedColorIntensityToRgb(smoothedHalftonePatterns);
  
  // Gamma correction (decode from linear to screen space).
  outputRgb = pow(outputRgb, vec3(1.0 / 2.2));
  outputColor = vec4(outputRgb, 1.0);
}
