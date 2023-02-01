#version 150

uniform sampler2D tex;
uniform vec2 dimensions;
uniform float time;
in vec2 coord;
out vec4 outputColor;

float sdOctahedron(vec3 p, float s)
{
  p = abs(p);
  float m = p.x + p.y + p.z - s;
  vec3 r = 3.0*p - m;
  
#if 0
  // filbs111's version (see comments)
  vec3 o = min(r, 0.0);
  o = max(r*2.0 - o*3.0 + (o.x+o.y+o.z), 0.0);
  return length(p - s*o/(o.x+o.y+o.z));
#else
  // my original version
  vec3 q;
  if( r.x < 0.0 ) q = p.xyz;
  else if( r.y < 0.0 ) q = p.yzx;
  else if( r.z < 0.0 ) q = p.zxy;
  else return m*0.57735027;
  float k = clamp(0.5*(q.z-q.y+s),0.0,s);
  return length(vec3(q.x,q.y-s+k,q.z-k));
#endif
}


float map( in vec3 pos )
{
  float rad = 0.1*(0.5+0.5*sin(time*2.0));
  return sdOctahedron(pos,0.5-rad) - rad;
}

// https://iquilezles.org/articles/normalsSDF
vec3 calcNormal( in vec3 pos )
{
  vec2 e = vec2(1.0,-1.0)*0.5773;
  const float eps = 0.0005;
  return normalize( e.xyy*map( pos + e.xyy*eps ) +
                   e.yyx*map( pos + e.yyx*eps ) +
                   e.yxy*map( pos + e.yxy*eps ) +
                   e.xxx*map( pos + e.xxx*eps ) );
}

void main()
{
  // camera movement
  float an = 0.5*(time-10.0);
  vec3 ro = vec3( 1.0*cos(an), 0.4, 1.0*sin(an) );
  vec3 ta = vec3( 0.0, 0.0, 0.0 );
  // camera matrix
  vec3 ww = normalize( ta - ro );
  vec3 uu = normalize( cross(ww,vec3(0.0,1.0,0.0) ) );
  vec3 vv = normalize( cross(uu,ww));
  
  
  
  vec3 tot = vec3(0.0);
  
  vec2 p = (-dimensions.xy + 2.0*coord)/dimensions.y;
  
  // create view ray
  vec3 rd = normalize( p.x*uu + p.y*vv + 1.5*ww );
  
  // raymarch
  const float tmax = 3.0;
  float t = 0.0;
  for( int i=0; i<256; i++ )
  {
    vec3 pos = ro + t*rd;
    float h = map(pos);
    if( h<0.0001 || t>tmax ) break;
    t += h;
  }
  
  
  // shading/lighting
  vec3 col = vec3(0.209, 0.22, 0.41);
  if( t<tmax )
  {
    vec3 pos = ro + t*rd;
    vec3 nor = calcNormal(pos);
    float dif = clamp( dot(nor,vec3(0.7,0.6,0.4)), 0.0, 1.0 );
    float amb = 0.5 + 0.5*dot(nor,vec3(0.0,0.8,0.6));
    col = vec3(0.2,0.3,0.4)*amb + vec3(0.8,0.7,0.5)*dif;
  }
  
  // gamma
  col = sqrt( col );
  tot += col;

  outputColor = vec4( tot, 1.0 );
}
