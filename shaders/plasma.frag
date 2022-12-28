#version 150

uniform sampler2DRect tex;
uniform float time;
uniform vec2 plasma;
uniform vec2 dimensions;
in vec2 coord;
out vec4 outputColor;

void main()
{
  
  float t = time/.1 + plasma.x;
  vec2 U = coord;
  vec2 R = dimensions.xy;
  vec2 S = vec2(160,100);
  vec2 p = ( U+U - R ) / R * S;
  vec2 q = vec2(cos(-t / 165.), cos( t / 45.))  * S - p;
  t = 1. + cos( length( vec2(cos( t / 98.),  sin( t / 178.)) * S - p ) / 30.) + cos( length( vec2(sin(-t / 124.), cos( t / 104.)) * S - p ) / 20.) + sin( length(q) / 25. ) * sin(q.x / 20.) * sin(q.y / 15.);
  outputColor = .5 + .5* cos( (time + plasma.y) / vec4(63,78,45,1) + ( t + vec4(0,1,-.5,0) ) *3.14 );
}
