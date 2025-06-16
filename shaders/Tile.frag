#version 150

uniform sampler2D tex;
uniform float repeat;
uniform float spacing;
uniform bool mirror;

in vec2 coord;
out vec4 outputColor;

void main()
{
  // Calculate the size of each tile including spacing
  float tileSize = 1.0 / repeat;
  float spacingSize = spacing / 100.0; // Convert spacing to a 0-1 range
  float effectiveTileSize = tileSize - spacingSize;
  
  // Calculate the tile coordinates with spacing
  vec2 tileCoord = mod(coord * repeat, 1.0);
  vec2 tileIndex = floor(coord * repeat);
  
  // Check if we're in the spacing area
  if (tileCoord.x > effectiveTileSize || tileCoord.y > effectiveTileSize) {
    outputColor = vec4(0.0, 0.0, 0.0, 1.0); // Black spacing
    return;
  }
  
  // Scale the coordinates to fit within the effective tile size
  vec2 repeatedTexCoord = tileCoord / effectiveTileSize;

  // Mirror the texture in the x coordinate if the mirror uniform is true
  if (mirror) {
    repeatedTexCoord.x = 1.0 - repeatedTexCoord.x;
  }
  
  // Sample the texture at the tiled texture coordinates
  outputColor = texture(tex, repeatedTexCoord);
}



