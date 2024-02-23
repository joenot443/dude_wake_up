#version 150

uniform sampler2D tex;

uniform vec2 outDimensions;
uniform vec2 inDimensions;
in vec2 coord;
out vec4 outputColor;

void main()
{
  float xScale = outDimensions.x / inDimensions.x;
  float yScale = outDimensions.y / inDimensions.y;
  
  // Sample the texture at the tiled texture coordinates
  outputColor = texture(tex, vec2(coord.x * xScale, coord.y * yScale));
}
