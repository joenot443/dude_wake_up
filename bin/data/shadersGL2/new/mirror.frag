#version 120

uniform sampler2DRect tex;
varying vec2 coord;
uniform vec2 dimensions;
uniform vec2 offset;

uniform int mirrorXEnabled;
uniform int mirrorYEnabled;

void main()
{
  vec4 tex_color=texture2DRect(tex, coord);
  
  
  // Normalized pixel coordinates (from 0 to 1)
  vec2 uv = coord / dimensions.xy;
  float z01 = step(0.5, uv.x);
  
  float z02 = step(0.5, uv.y);
  
  
  vec2 col = vec2(coord);
  
  // Mirror if enabled
  if (mirrorXEnabled == 1) {
    col.x = (abs(uv.x - z01) + offset.x) * dimensions.x;
  }
  
  if (mirrorYEnabled == 1) {
    col.y = (abs(uv.y - z02) + offset.y) * dimensions.y;
  }
  
  // Output to screen
  gl_FragColor = texture2DRect(tex, col);
  
}
