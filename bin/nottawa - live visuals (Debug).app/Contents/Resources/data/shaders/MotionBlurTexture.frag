#version 150

uniform sampler2D tex;
uniform vec2 dimensions;
uniform float time;
in vec2 coord;
out vec4 outputColor;


vec3 deform( in vec2 p, in float t )
{
  t *= 2.0;
  
  p += 0.5*sin( t*vec2(1.1,1.3)+vec2(0.0,0.5) );
  
  float a = atan( p.y, p.x );
  float r = length( p );
  
  float s = r * (1.0+0.5*cos(t*1.7));
  
  vec2 uv = 0.1*t + 0.05*p.yx + 0.05*vec2( cos(t+a*2.0),
                                          sin(t+a*2.0))/s;
  
  return texture( tex, 0.5*uv ).xyz;
}

void main()
{
  vec2 q = coord/dimensions.xy;
  vec2 p = -1.0 + 2.0*q;
  
  vec3 col = vec3(0.0);
  for( int i=0; i<20; i++ )
  {
    float t = time + float(i)*0.0035;
    col += deform( p, t );
  }
  col /= 20.0;
  
  col = pow( col, vec3(0.6,0.7,0.8) );
  
  outputColor = vec4( col, 1.0 );
}
