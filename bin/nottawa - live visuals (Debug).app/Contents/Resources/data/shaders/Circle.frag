#version 150

uniform sampler2D tex;
uniform vec2 dimensions;
uniform float time;
in vec2 coord;
out vec4 outputColor;


void main() {
  float circleRadius = 100.0;
  vec2 center = vec2(dimensions.x / 2.0, dimensions.y / 2.0);
  vec2 uv = coord.xy;
  
  float distanceFromCenter = distance(uv, center);
  float circleAlpha = 1.0 - clamp(distanceFromCenter / circleRadius, 0.0, 1.0);
  
  vec4 circleColor = vec4(1.0, 1.0, 1.0, 1.0); // Bluish color
  vec4 backgroundColor = vec4(0.0, 0.0, 0.0, 0.0); // Transparent black
  
  vec4 finalCircleColor = vec4(circleColor.rgb, circleColor.a * circleAlpha);
  outputColor = mix(backgroundColor, finalCircleColor, circleAlpha);
}
