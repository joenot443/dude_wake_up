#version 150

// Input uniforms
uniform sampler2D tex;
uniform float time;
uniform vec2 dimensions;
uniform float rotationAngle;
uniform float patternDensity;
uniform vec3 colorCycle;
uniform float timeScale;

in vec2 coord;
out vec4 outputColor;

void main(void) {
  // Normalize and center UV coordinates
  vec2 uv = (coord.xy - dimensions.xy * .5) / dimensions.y;

  // Create rotation matrix from parameter
  float cosRot = cos(rotationAngle);
  float sinRot = sin(rotationAngle);
  uv *= mat2(cosRot, -sinRot, sinRot, cosRot);
  
  // Scale up for pattern repetition using density parameter
  uv *= patternDensity;

  // Get grid cell coordinates and fractional offset
  vec2 gridCell = floor(uv);
  vec2 cellOffset = fract(uv) - .5;

  // Initialize color mixing vector with color cycle influence
  vec3 colorMix = vec3(.5, 1, 0) * (0.5 + 0.5 * colorCycle);
  float patternTime;

  // Iterate through 3x3 grid of neighboring cells
  for (float y = -1.; y <= 1.; y++) {
    for (float x = -1.; x <= 1.; x++) {
      vec2 neighborOffset = vec2(x, y);

      // Calculate time-based pattern with time scale
      patternTime = -time * timeScale + length(gridCell - neighborOffset) * .2;
      
      // Use color cycle to influence the radius variation
      float radius = mix(.4, 1.5, sin(patternTime + colorCycle.x * 2.0) * .5 + .5);
      float circle = smoothstep(radius, radius * .9, length(cellOffset + neighborOffset));
      
      // Mix colors based on circle pattern with color cycle influence
      vec3 cycleInfluence = vec3(
        sin(patternTime + colorCycle.x * 3.0),
        sin(patternTime + colorCycle.y * 3.0),
        sin(patternTime + colorCycle.z * 3.0)
      ) * 0.5 + 0.5;
      
      colorMix.zyx = colorMix.yxz * (1. - circle) + circle * (1. - colorMix.xzy) * cycleInfluence;
    }
  }

  outputColor = vec4(colorMix, 1);
}
