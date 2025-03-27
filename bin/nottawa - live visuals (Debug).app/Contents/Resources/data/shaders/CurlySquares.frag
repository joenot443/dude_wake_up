#version 150

uniform sampler2D tex;
uniform vec2 dimensions;
uniform float time;
uniform vec3 color;
uniform float size;
uniform float alpha;
uniform float beta;
uniform float gamma;
in vec2 coord;
out vec4 outputColor;

#define pi 3.14159265359

#define float2 vec2
#define float3 vec3
#define float4 vec4

float saw(float x)
{
  return abs(fract(x)-0.5)*2.0;
}

float dw(float2 p, float2 c, float t)
{
  return sin(length(p-c)-t);
}

float dw1(float2 uv)
{
  float v=0.0;
  float t=time*2.0 * alpha; // Modified by alpha
  v+=dw(uv,float2(sin(t*0.07 + beta)*30.0,cos(t*0.04)*20.0),t*1.3);
  v+=dw(uv,float2(cos(t*0.13)*30.0,sin(t*0.14 + beta)*20.0),t*1.6+1.0);
  v+=dw(uv,float2( 18,-15),t*0.7+2.0);
  v+=dw(uv,float2(-18, 15),t*1.1-1.0);
  return v/4.0;
}

float fun(float x, float y)
{
  return dw1(float2(x-0.5,y-0.5)*80.0);
}

vec3 duv(float2 uv)
{
  float x=uv.x*size;
  float y=uv.y*size;
  float v=fun(x,y);
  
  float d=1.0/400.0;
  float dx=(v-fun(x+d,y))/d;
  float dy=(v-fun(x,y+d))/d;
  float a=atan(dx,dy)/pi/2.0;
  return float3(v,0,(v*gamma+a));
}

void main()
{
  vec2 uv = coord.xy/dimensions.x;
  vec3 h=duv(uv);
  float sp=saw(h.z+time*1.3);
  
  sp=clamp((sp-0.25)*2.0,0.5,1.0);
  outputColor = float4((h.x+0.5*(1.0 - color.x))*sp*alpha, (0.3+saw(h.x+0.5*(1.0 - color.y))*0.6)*sp*beta, (0.6-h.x)*sp, 1.0);
}
