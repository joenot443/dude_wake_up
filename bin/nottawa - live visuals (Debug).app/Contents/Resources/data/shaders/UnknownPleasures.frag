#version 150

uniform sampler2D tex;
uniform vec2 dimensions;
uniform float time;
uniform float height;
in vec2 coord;
out vec4 outputColor;

float hash( uint n )
{   // integer hash copied from Hugo Elias
  n = (n<<13U)^n;
  n = n*(n*n*15731U+789221U)+1376312589U;
  return float(n&uvec3(0x0fffffffU))/float(0x0fffffff);
}

float bnoise( in float x )
{// Basic noise copied from iq
  float i = floor(x);
  float f = fract(x);
  float k = fract(i*.1731);
  return f*(f-1.0)*((18.0*k-4.0)*f*(f-1.0)-1.0);
}


float dist(vec2 uv){
  float w = 0.007; //width of each line
  uvec2 m = uvec2((0.27/w), (0.71/w)); //uv.y min and max
  uint id = uint(uv.y /w); //id of each line to get a random hash
  for(uint i = max(m.x, id-10u); i <=min(id, m.y); ++i)
  {
    float h = hash(i);
    float s = 2.0*h*time + h;
    float x = uv.x*10.0;
    float ob = bnoise(x*1.5 + s);
    float od = bnoise(x*8. + 4.0*s);
    float f = max(sin(x*1.5+0.3),0.);
    float o =w + f*0.06*h*(f-ob-0.2*od) +0.002*od;
    float nd = uv.y - (float(i) * w) - o;
    if(nd < 0.001) return nd;
  }
  return 1.0;
}


void main(  )
{
  vec2 uv = (coord -0.5*dimensions.xy)/min(dimensions.y, dimensions.x);
  uv = (uv + 1.0)/2.0;
  float a = float(uv.x > 0.3 && uv.x < 0.7);
  a*= float(abs(dist(uv)) <= 0.0008);
  outputColor = vec4(mix(0.1, 0.9, a));
  
}
