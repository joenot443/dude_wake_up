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
  float circleAlpha = step(distanceFromCenter, circleRadius);
  
  vec4 circleColor = vec4(0.5, 0.6, 0.8, 1.0); // Bluish color
  vec4 backgroundColor = vec4(0.0, 0.0, 0.0, 0.0); // Transparent black
  
  outputColor = mix(backgroundColor, circleColor, circleAlpha);
}
