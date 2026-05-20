#version 150

uniform sampler2D tex;
uniform vec2 dimensions;
uniform float time;
uniform float cellCount;
uniform vec4 backgroundColor;
in vec2 coord;
out vec4 outputColor;

#define S(v)  smoothstep( -n/R.y, n/R.y, v )                       // AA draw

void main()
{
  float n = cellCount;                                              // number of cells in height
  float v = n;
  vec2 R = dimensions.xy;
  vec2 U = n * coord / R.y;                                           // normalized coordinates
  vec2 I = floor(U);                                             // cell Id
  vec2 P = coord+coord - R;                                              // centered coordinates
  vec2 F = (U-I-.5) * mat2(cos(atan(P.y,P.x)+vec4(0,11,33,0)));  // local coords oriented toward center
  v = .3 + .6 * texture(tex, I/n*R.y/R).r;                 // cell value to display
  float crescent = S( (v*2.-1.) * sqrt(1.-4.*F.y*F.y) - F.x );    // moon crescent(v)
  float mask = S( length(F)-.45 );                                // sphere mask
  outputColor = mix( vec4(vec3(crescent), 1.0), backgroundColor, mask );
}
