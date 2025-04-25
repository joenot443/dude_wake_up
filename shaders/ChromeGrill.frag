#version 150

uniform sampler2D tex;
uniform vec2 dimensions;
uniform float time;
uniform float color;
uniform float warp;
in vec2 coord;
out vec4 outputColor;

// Chrome Grill by @felixturner

#define PI 3.1415926535

const float LINE_THICKNESS = 0.2;
const float SPEED = 0.262;
const float DISPLACE = 0.0001;

//const float STRIPE_COUNT = 50.0;

// --------------------------
//VALUE NOISE - CHEAPER
//from: https://www.shadertoy.com/view/lsf3WH

float hashV( vec2 p )
{
  float h = dot(p,vec2(127.1,311.7));
  return -1.0 + 2.0*fract(sin(h)*43758.5453123);
}

float noiseV( in vec2 p )
{
  vec2 i = floor( p );
  vec2 f = fract( p );
  
  vec2 u = f*f*(3.0-2.0*f);
  
  return mix( mix( hashV( i + vec2(0.0,0.0) ),
                  hashV( i + vec2(1.0,0.0) ), u.x),
             mix( hashV( i + vec2(0.0,1.0) ),
                 hashV( i + vec2(1.0,1.0) ), u.x), u.y);
}

//-----------------------

vec2 rotate2D(vec2 position, float theta)
{
  mat2 m = mat2( cos(theta), -sin(theta), sin(theta), cos(theta) );
  return m * position;
}

//draw stripes along x axis
//returns 1 for on line, 0 otherwise
float stripes(vec2 st){
  
  float STRIPE_COUNT = 30.;//dimensions.x/30.; //resolution independent stripe width
  
  st = fract( st * STRIPE_COUNT);
  float pos = 0.5;
  return smoothstep( pos-LINE_THICKNESS, pos,  st.y) -
  smoothstep( pos, pos+LINE_THICKNESS,  st.y);
  
}

//warp uv space by noise value
vec2 distort(in vec2 uv , float time, float amount)
{
  vec2 uv2 = uv + vec2(time);
  uv2 *= 1.8;  //scale noise
  uv += vec2(noiseV(uv2) * amount);
  return uv;
}


void main(  )
{
  vec2 uv = vec2( coord.xy - 0.5*dimensions.xy ) / min(dimensions.x,dimensions.y);
  
  float time = time * SPEED;
  
  //rotate
  uv = rotate2D(uv,time/10.1);
  
  uv = rotate2D(uv, PI/2.);
  
  //uv.y += time/6.;
  
  //sin warp on mouse X
  uv.y = uv.y + sin(uv.x * 6. + time * 6. )*warp/3. + cos(uv.x * 3. + time / 2.)*warp/6.0;
  
  //white background
  vec3 col = vec3(1.);
  
  float OFFSET = 1./60.;
  
  //draw rgb seperately with a slight time offset
  for (int i = 0; i < 3; i++) {
    float time2 = time + float(i) * DISPLACE;
    uv = distort(uv, time2,OFFSET);
    col[i] -=stripes(uv);
  }
  
  //black stripes on top
  uv = distort(uv, time, color/30.);
  col -=stripes(uv);
  
  outputColor = vec4( col, 1.0 );
}
