#version 150

uniform sampler2D tex;
in vec2 coord;
out vec4 outputColor;
uniform vec2 dimensions;
uniform vec2 offset;

uniform int mirrorXEnabled;
uniform int mirrorYEnabled;

void main()
{
  vec4 tex_color=texture(tex, coord);
  
  
  float z01 = step(0.5, coord.x);
  float z02 = step(0.5, coord.y);
  
  
  vec2 col = vec2(coord);
  
  // Mirror if enabled
  if (mirrorXEnabled == 1) {
    col.x = (abs(coord.x - z01) + offset.x);
  }
  
  if (mirrorYEnabled == 1) {
    col.y = (abs(coord.y - z02) + offset.y);
  }
  
  // Output to screen
  outputColor = texture(tex, col);
  
}
