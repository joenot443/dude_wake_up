#version 150

// Input uniforms
uniform sampler2D tex;        // Source texture
uniform vec2 dimensions;      // Screen dimensions
uniform float time;          // Current time
uniform float speed;         // Animation speed multiplier
uniform float redMultiplier;
uniform float greenMultiplier;
uniform float blueMultiplier;
in vec2 coord;               // UV coordinates
out vec4 outputColor;        // Final output color

void main()
{
  // Scale time for slower animation
  float scaledTime = time * 0.20;
  
  // Convert coordinates to UV space
  vec2 uv = coord.xy / dimensions.xy;
  vec2 position = uv;
  
  // Calculate polar coordinates
  float radius = sqrt(dot(position, position));
  float angle = atan(position.y, position.x) + scaledTime * speed;
  
  // Convert back to cartesian with rotation
  vec2 rotatedPos = vec2(cos(angle), sin(angle)) * radius;
  
  // Sample texture with slight time-based offset
  vec4 color = texture(tex, rotatedPos + vec2(scaledTime * 0.01, 0.0));
  
  // Apply color channel oscillations
  color.r = sin(color.r * 25.0 * redMultiplier + scaledTime * 15.0);
  color.g = sin(color.g * 30.0 * greenMultiplier + scaledTime * 30.0);
  color.b = sin(color.b * 30.0 * blueMultiplier + scaledTime * 10.0);
  
  outputColor = color;
}

