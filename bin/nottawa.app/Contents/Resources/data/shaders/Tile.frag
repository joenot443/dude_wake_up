#version 150

uniform sampler2D tex;
uniform float repeat;
uniform bool mirror;

in vec2 coord;
out vec4 outputColor;

void main()
{
  // Tile the texture into an 8x8 grid
  vec2 repeatedTexCoord = coord * repeat;

  // Mirror the texture in the x coordinate if the mirror uniform is true
  if (mirror) {
    repeatedTexCoord.x = 1.0 - repeatedTexCoord.x;
  }
  
  // Sample the texture at the tiled texture coordinates
  outputColor = texture(tex, repeatedTexCoord);
}



