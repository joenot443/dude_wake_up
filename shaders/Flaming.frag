#version 150

// Input uniforms
uniform sampler2D tex;
uniform vec2 dimensions;
uniform float time;
in vec2 coord;
out vec4 outputColor;

// CC0: Another flaming experiment
//  Doesn't look like real flames but I think it looks neat anyway

void main() {
  // Core vectors for ray marching
  vec2 resolution = dimensions;  // Screen dimensions
  vec3 colorAccum = vec3(0.0);  // Accumulated color output
  vec3 rayPos = vec3(0.0);      // Current ray position
  vec3 basePos = vec3(0.0);     // Reference position for calculations
  
  // Ray marching loop parameters
  float stepCount = 0.0;        // Ray step counter
  float turbulence = 0.0;       // Turbulence accumulator
  float distToSurface = 0.0;    // SDF result - distance to nearest surface
  float rayDepth = 0.0;         // Current depth along ray
  float currentTime = time;     // Animation time
  
  // Main ray marching loop
  for (; ++stepCount < 88.0; rayDepth += distToSurface/8.0) {
    // Calculate ray direction from camera through pixel
    rayPos = rayDepth * normalize(vec3(coord - 0.5 * resolution.xy, resolution.y));
    rayPos.z -= 4.0;
    basePos = rayPos;
    
    // Apply space transformation
    rayPos.xz *= mat2(cos(basePos.y * basePos.y / 4.0 + 2.0 * basePos.y - currentTime + vec4(0, 11, 33, 0)));
    rayPos.x += sin(0.2 * currentTime - basePos.x);
    
    // Apply turbulence using fractal noise
    distToSurface = turbulence = 9.0;
    for(; --turbulence > 5.0; distToSurface /= 0.8) {
      rayPos += 0.4 * (rayPos.y + 2.0) * cos(rayPos.zxy * distToSurface - 3.0 * currentTime) / distToSurface;
    }
    
    // Calculate distance field
    turbulence = length(rayPos - basePos);
    rayPos = abs(rayPos);
    
    // Compute intersection of two tapered boxes
    distToSurface = abs(
      min(
        max(rayPos.z - 0.1, rayPos.x - 1.0 - 0.3 * basePos.y),
        max(rayPos.x - 0.2, rayPos.z - 1.0 - 0.3 * basePos.y)
      )
    ) + 0.009;  // Small offset for translucency
    
    // Generate flame colors
    basePos = 1.0 + sin(0.5 + turbulence - basePos.y + basePos.z + vec3(2, 3, 4));
    
    // Accumulate color with intensity falloff
    colorAccum += basePos.x / distToSurface * basePos;
  }
  
  // Apply tone mapping and output final color
  outputColor = tanh(colorAccum.xyzx / 2000.0);
}
