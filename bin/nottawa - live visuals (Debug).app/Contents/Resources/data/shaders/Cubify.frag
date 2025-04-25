#version 150

// --- Uniforms, Inputs, Outputs (Names Preserved) ---
uniform sampler2D tex;          // Input texture sampler
uniform vec2 dimensions;        // Screen/render target dimensions
uniform float time;             // Time uniform for animation
uniform float cubeSize;         // Controls the apparent size of the grid cells/cubes
in vec2 coord;                  // Input fragment coordinate (pixel coordinate)
out vec4 outputColor;           // Final fragment color output

// --- Pseudo-Random Number Generation (PCG) ---

// PCG hash function for 3D unsigned integer vector.
uvec3 pcg3d_hash(uvec3 state)
{
  state = state * 1664525u + 1013904223u;
  state.x += state.y * state.z; state.y += state.z * state.x; state.z += state.x * state.y;
  state = state ^ (state >> 16u);
  state.x += state.y * state.z; state.y += state.z * state.x; state.z += state.x * state.y;
  return state;
}

// Converts a 3D float vector to a 3D float random vector in [0, 1] range using PCG hash.
vec3 pcg3d_random(vec3 position)
{
  // Use absolute value with offset to ensure positive input for uvec3 conversion
  uvec3 unsignedInput = uvec3(abs(position + 10000.0));
  uvec3 randomUnsigned = pcg3d_hash(unsignedInput);
  // Convert lower 16 bits of each component to float in [0, 1]
  return vec3(randomUnsigned & uvec3(0xffffu)) * (1.0 / 65535.0);
}

// --- Raymarching/Intersection Utilities ---

// Calculates the intersection distances (tNear, tFar) of a ray with a unit box centered at the origin.
vec2 intersectUnitBox(vec3 rayOrigin, vec3 rayDirection)
{
  vec3 invDirection = 1.0 / rayDirection;
  vec3 tMinPlanes = (vec3(-1.0) - rayOrigin) * invDirection;
  vec3 tMaxPlanes = (vec3( 1.0) - rayOrigin) * invDirection;
  vec3 t1 = min(tMinPlanes, tMaxPlanes);
  vec3 t2 = max(tMinPlanes, tMaxPlanes);
  float tNear = max(max(t1.x, t1.y), t1.z);
  float tFar = min(min(t2.x, t2.y), t2.z);
  return vec2(tNear, tFar);
}

// Calculates the normal vector of a unit box at a given surface point p.
vec3 normalUnitBox(vec3 surfacePoint)
{
  vec3 absPoint = abs(surfacePoint);
  // Selects the axis (x, y, or z) with the largest absolute value
  bvec3 selectionMask = greaterThan(absPoint.xyz, max(absPoint.yxx, absPoint.zzy));
  // Returns sign(p) along the selected axis, zero otherwise
  return mix(vec3(0.0), sign(surfacePoint), selectionMask);
}

// --- Rotation Matrices ---

// Creates a 3x3 rotation matrix around the X-axis.
mat3x3 rotationMatrixX(float angle)
{
  float s = sin(angle);
  float c = cos(angle);
  return mat3x3(
                1.0, 0.0, 0.0,
                0.0, c,  -s,
                0.0, s,   c
                );
}

// Creates a 3x3 rotation matrix around the Y-axis.
mat3x3 rotationMatrixY(float angle)
{
  float s = sin(angle);
  float c = cos(angle);
  return mat3x3(
                c,   0.0, s,
                0.0, 1.0, 0.0,
                -s,  0.0, c
                );
}

// Creates a 3x3 rotation matrix around the Z-axis.
mat3x3 rotationMatrixZ(float angle)
{
  float s = sin(angle);
  float c = cos(angle);
  return mat3x3(
                c,  -s,  0.0,
                s,   c,  0.0,
                0.0, 0.0, 1.0
                );
}

// --- Cube Ray Tracing ---

// Traces a ray against a rotated unit cube and calculates simple diffuse lighting.
// Note: backgroundColor parameter is currently unused within this function.
vec3 traceCube(vec2 localUV, vec3 rotationAngles, vec3 cubeDiffuseColor, vec3 backgroundColor)
{
  // Define initial ray direction (along -Z) and origin
  vec3 initialRayDir = vec3(0.0, 0.0, -1.0);
  // Scale UVs and set initial Z position for perspective effect
  vec3 initialRayPos = vec3(localUV * 1.7, 2.0);
  
  // Combine rotations
  mat3x3 rotationMat = rotationMatrixX(rotationAngles.x) * rotationMatrixY(rotationAngles.y) * rotationMatrixZ(rotationAngles.z);
  mat3x3 inverseRotationMat = inverse(rotationMat); // Needed for transforming normal back
  
  // Transform ray into cube's object space
  vec3 rotatedRayDir = initialRayDir * rotationMat;
  vec3 rotatedRayPos = initialRayPos * rotationMat;
  
  // Intersect transformed ray with the unit box
  vec2 intersectionResult = intersectUnitBox(rotatedRayPos, rotatedRayDir);
  
  vec3 resultColor;
  // Check if the intersection is valid (tNear < tFar and tNear > 0 implicitly checked by max())
  if (intersectionResult.x < intersectionResult.y)
  {
    // Hit occurred
    // Adjust input color slightly (original behavior)
    vec3 adjustedDiffuseColor = cubeDiffuseColor + vec3(0.05);
    
    // Calculate hit position in object space
    vec3 hitPositionObjectSpace = rotatedRayPos + intersectionResult.x * rotatedRayDir;
    
    // Calculate normal in object space and transform back to world space
    vec3 normalObjectSpace = normalUnitBox(hitPositionObjectSpace);
    vec3 normalWorldSpace = normalObjectSpace * inverseRotationMat;
    
    // Define two fixed directional lights
    const vec3 lightDir0 = normalize(vec3( 0.1,  0.3, 1.0)); // Main light
    const vec3 lightDir1 = normalize(vec3(-0.6, -0.3, 0.2)); // Fill light
    
    // Calculate simple diffuse lighting (Lambertian)
    float lambertian0 = max(dot(lightDir0, normalWorldSpace), 0.0);
    float lambertian1 = max(dot(lightDir1, normalWorldSpace), 0.0);
    
    vec3 lightingContribution = vec3(1.0) * lambertian0;         // Main light contribution (white)
    lightingContribution += vec3(0.2, 0.2, 0.7) * lambertian1; // Fill light contribution (blueish)
    
    resultColor = adjustedDiffuseColor * lightingContribution;
  }
  else
  {
    // No hit
    resultColor = vec3(0.0); // Return black
  }
  
  return resultColor;
}

// --- Main Shader ---
void main()
{
  // Normalize pixel coordinates to UV space [0, 1]
  vec2 screenUV = coord / dimensions.xy;
  
  // Convert screen UV to normalized device coordinates (NDC) [-1, 1], adjusting for aspect ratio
  float aspectRatio = dimensions.x / dimensions.y;
  vec2 ndc = screenUV * 2.0 - 1.0;
  ndc.x *= aspectRatio;
  
  // Calculate a scaling factor for the grid based on screen width and cubeSize uniform
  // Smaller cubeSize -> larger gridScaleFactor -> smaller apparent cells
  float gridPixelSize = dimensions.x / 140.0; // Base size for 140 cells across width
  float gridScaleFactor = gridPixelSize / cubeSize;
  
  // Determine the UV coordinates of the bottom-left corner of the current grid cell
  vec2 gridOriginUV = floor(gridScaleFactor * ndc) / gridScaleFactor;
  
  // Calculate the UV coordinate within the current grid cell, scaled to [-1, 1]
  vec2 localUVInCell = ((ndc - gridOriginUV) * gridScaleFactor) * 2.0 - 1.0;
  
  // Sample background color from input texture (highly blurred)
  vec3 backgroundColorSample = textureLod(tex, screenUV, 9.0).xyz;
  // Sample cube color from input texture based on grid cell origin (less blurred)
  vec3 cubeColorSample = textureLod(tex, gridOriginUV * vec2(0.5 / aspectRatio, 0.5) + 0.5, 4.0).xyz;
  
  // Determine rotation angles based on cube color sample and time
  vec3 rotationAngles = cubeColorSample * 3.0;
  rotationAngles.z += time * 2.5; // Add time-based rotation around Z
  
  // Gamma correction for input colors (assuming input texture is sRGB)
  vec3 cubeColorLinear = pow(cubeColorSample, vec3(2.2));
  vec3 backgroundColorLinear = pow(backgroundColorSample, vec3(2.2)); // Although not used in traceCube
  
  // Trace the ray for the current grid cell
  vec3 finalColorLinear = traceCube(localUVInCell, rotationAngles, cubeColorLinear, backgroundColorLinear);
  
  // Gamma correct the final color before outputting (convert linear back to sRGB)
  outputColor = vec4(pow(finalColorLinear, vec3(1.0 / 2.2)), 1.0);
}
