#version 150

#define DOTSIZE 1.48
#define D2R(d) radians(d)
#define MIN_S 0.5
#define MAX_S 19.0

#define SST 0.888
#define SSQ 0.288

uniform sampler2D tex;
uniform vec2 dimensions;
uniform float time;
uniform float amount;
uniform float speed;
uniform float rotate;
uniform float radius;

in vec2 coord;
out vec4 outputColor;

#define ORIGIN (0.5 * dimensions.xy)
float R;
float S;

vec4 rgb2cmyki(in vec3 c)
{
  float k = max(max(c.r, c.g), c.b);
  return min(vec4(c.rgb / k, k), 1.0);
}

vec3 cmyki2rgb(in vec4 c)
{
  return c.rgb * c.a;
}

vec2 px2uv(in vec2 px)
{
  return vec2(px / dimensions.xy);
}

vec2 grid(in vec2 px)
{
  return px - mod(px,S);
}

vec4 ss(in vec4 v)
{
  return smoothstep(SST-SSQ, SST+SSQ, v);
}

vec4 halftone(in vec2 fc,in mat2 m)
{
  vec2 smp = (grid(m*fc) + 0.5*S) * m;
  float s = min(length(fc-smp) / (DOTSIZE*0.5*S), 1.0);
  vec3 texc = texture(tex, px2uv(smp+ORIGIN)).rgb;
  texc = pow(texc, vec3(2.2)); // Gamma decode.
  vec4 c = rgb2cmyki(texc);
  return c+s;
}

mat2 rotm(in float r)
{
  float cr = cos(r);
  float sr = sin(r);
  return mat2(
              cr,-sr,
              sr,cr
              );
}

void main(  )
{
  if (speed < 0.1) {
    S = radius + MIN_S + (MAX_S-MIN_S) * 2.0*abs(0.5 * ORIGIN.x) / dimensions.x;
    R = D2R(180.0 * (0.5 * ORIGIN.y) / dimensions.y) * rotate;
  } else {
    R = speed*0.333*time;
    S = MIN_S + (MAX_S-MIN_S) * (0.5 - 0.5*cos(time*speed));
  }
  
  vec2 fc = coord.xy - ORIGIN;
  mat2 mc = rotm(R + D2R(15.0));
  mat2 mm = rotm(R + D2R(75.0));
  mat2 my = rotm(R);
  mat2 mk = rotm(R + D2R(45.0));
  
  float k = halftone(fc, mk).a;
  vec3 c = cmyki2rgb(ss(vec4(
                             halftone(fc, mc).r,
                             halftone(fc, mm).g,
                             halftone(fc, my).b,
                             halftone(fc, mk).a
                             )));
  
  c = pow(c, vec3(1.0/2.2)); // Gamma encode.
  outputColor = vec4(c, 1.0);
}
