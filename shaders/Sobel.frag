#version 150

// Input uniforms
uniform sampler2D tex;
uniform vec2 dimensions;
uniform float time;
uniform float tolerance;
uniform float width;  // Controls edge width (1.0 = default, higher = wider)

// Vertex shader outputs
in vec2 coord;
out vec4 outputColor;

void main() {
  // Normalize coordinates to [0,1] range
  vec2 uv = coord.xy / dimensions.xy;
  vec3 finalColor;

  // Sobel edge detection kernels
  // Note: GLSL's mat3 is COLUMN-major -> mat3[col][row]
  mat3 sobelX = mat3(
    -1.0, -2.0, -1.0,
     0.0,  0.0,  0.0,
     1.0,  2.0,  1.0
  ) * tolerance;

  mat3 sobelY = mat3(
    -1.0,  0.0,  1.0,
    -2.0,  0.0,  2.0,
    -1.0,  0.0,  1.0
  ) * tolerance;

  float gradientX = 0.0;  // Horizontal edge strength
  float gradientY = 0.0;  // Vertical edge strength

  // Apply convolution
  for (int y = -1; y <= 1; y++) {
    for (int x = -1; x <= 1; x++) {
      // Calculate texture coordinates for current kernel position
      float texX = (coord.x + float(x)) / dimensions.x;
      float texY = (coord.y + float(y)) / dimensions.y;
      
      // Sample and apply kernel weights
      vec3 sampleColor = texture(tex, vec2(texX, texY)).xyz;
      float luminance = length(sampleColor);
      
      gradientX += luminance * float(sobelX[1 + x][1 + y]);
      gradientY += luminance * float(sobelY[1 + x][1 + y]);
    }
  }

  // Calculate final edge strength
  float edgeStrength = abs(gradientX) + abs(gradientY);
  // Alternative: edgeStrength = sqrt(gradientX * gradientX + gradientY * gradientY);

  // Threshold the result
  float edge = (edgeStrength > 1.0) ? 1.0 : 0.0;
  
  // Dilate edges based on width parameter
  float maxEdge = edge;
  if (width > 1.0) {
    int radius = int(width);
    for (int y = -radius; y <= radius; y++) {
      for (int x = -radius; x <= radius; x++) {
        float texX = (coord.x + float(x)) / dimensions.x;
        float texY = (coord.y + float(y)) / dimensions.y;
        
        // Calculate gradient at neighboring pixel
        float neighborGradientX = 0.0;
        float neighborGradientY = 0.0;
        
        for (int ky = -1; ky <= 1; ky++) {
          for (int kx = -1; kx <= 1; kx++) {
            float sampleX = texX + float(kx) / dimensions.x;
            float sampleY = texY + float(ky) / dimensions.y;
            vec3 sampleColor = texture(tex, vec2(sampleX, sampleY)).xyz;
            float luminance = length(sampleColor);
            
            neighborGradientX += luminance * float(sobelX[1 + kx][1 + ky]);
            neighborGradientY += luminance * float(sobelY[1 + kx][1 + ky]);
          }
        }
        
        float neighborEdge = (abs(neighborGradientX) + abs(neighborGradientY) > 1.0) ? 1.0 : 0.0;
        maxEdge = max(maxEdge, neighborEdge);
      }
    }
  }

  finalColor = vec3(maxEdge);
  outputColor.xyz = finalColor;
}
