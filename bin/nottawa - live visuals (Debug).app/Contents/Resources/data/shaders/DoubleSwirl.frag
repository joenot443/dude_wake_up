#version 150

uniform sampler2D tex;
uniform vec2 dimensions;
uniform float time;
in vec2 coord;
out vec4 outputColor;

// golfed variant of 1982/ 1759 chars   https://shadertoy.com/view/llsfRj
// ( See original for comments )

#define f(d) fract( d(U) + .5 ) - .5//
#define t time/4.//
#define d(l) smoothstep( 2./R.y, 0., length( p/2. - D( vec2( floor( i + U.x*l ) / l , U.y ) )) -.005 )

mat2 M = mat2(5,1, .628,-.5);

vec2 D(vec2 q)
{
  q = inverse(M) * ( q - vec2(-t, t) ) ;
  float a = 3.125,
  l = dot( q = exp(q.y) * cos( (q.x - t/8.) *6.283 + vec2(0,11)) -.5 , q ),
  v = 1.7225 / ( 1.+ l -q-q ).x,
  b = ( v - v*l + .885 ) / .8,
  A = 1. +  a*a,
  B = ( b*a -.765625 ) / A,  // could del max, but some rare negatives
  x = sign(q.y)* sqrt( max(0., B*B - ( b*b -1.05*b + 1.04125 - v*l ) / A ) ) - B;
  return vec2( x , -b - a*x );
}

void main()
{
  vec2 R = dimensions.xy,
  p = ( coord+coord - R ) / R.y,
  z = p + vec2(1.75,1.05), q = p - vec2(.75, -.25);
  q = mat2(z,z.y,-z) * q / dot(q,q) + .5;
  
  vec2 U = M * vec2( atan(q.y,q.x)/6.283 + t/8.,
               log( length(q) )  )
  + vec2(-t, t);
  
  float l = log2( 20.* length( inverse(mat2(f(dFdx),f(dFdy)))[1] )/ max(360.,R.y) ),
  a = fract(l), i = 0.;
  l = exp2(l-a);
  
  outputColor += mix( d(l), d((l+l)), a );
  outputColor += mix( d(l), d((l+l)), a );
}
