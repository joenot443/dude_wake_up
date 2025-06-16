#version 150

// Input texture and dimensions
uniform sampler2D tex;
uniform vec2 dimensions;
uniform float time;
uniform float alpha;
uniform float gamma;
uniform float delta;

// Vertex shader outputs
in vec2 coord;
out vec4 outputColor;

void main()
{
  // Normalize coordinates to [0,1] range
  vec2 uv = coord.xy / dimensions.xy;
  
  // Center coordinates around origin
  vec2 centeredPos = (uv.xy - 0.5);
  
  // Calculate plasma pattern using sine waves and position
  vec2 plasmaPattern = (
    centeredPos.xy * centeredPos.xy + 
    sin(uv.x * 18.0 * gamma + time) / 25.0 * 
    sin(uv.y * 7.0 * delta + time * 1.5) / 1.0
  ) + 
  uv.x * sin(time) / 16.0 + 
  uv.y * sin(time * 1.2) / 16.0;
  
  // Generate circular pattern
  float circlePattern = sqrt(abs(plasmaPattern.x + plasmaPattern.y * 0.5) * 25.0) * 5.0;
  
  // Mix between user color and generated plasma color
  vec3 finalColor = vec3(
                         sin(circlePattern * 1.25 * alpha + 2.0),
                         abs(sin(circlePattern * 1.0 - 1.0) - sin(circlePattern)),
                         abs(sin(circlePattern) * 1.0 * (1.0 - 0.5*alpha))
                         );
  
  outputColor = vec4(finalColor, 1.0);
}
