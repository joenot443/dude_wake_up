#version 150

uniform sampler2DRect tex;
in vec2 coord;
uniform float sides;
uniform float shift;
uniform float rotation;
uniform vec2 dimensions;
out vec4 outputColor;

void main()
{
  vec2 position = coord.xy / dimensions.xy;
	  // normalize to the center
  vec2 p = position - 0.5;
  
  // cartesian to polar coordinates
  float r = length(p);
  float a = atan(p.y, p.x);
  
  // kaleidoscope
  float tau = rotation * 3.14159;
  a = mod(a, tau/sides);
  a = abs(a - tau/sides/2.);
  
  // polar to cartesian coordinates
  p = (r * vec2(cos(a), sin(a)) + shift)  * dimensions.xy;
  
  vec4 color = texture(tex, p);
  outputColor = color;
}
