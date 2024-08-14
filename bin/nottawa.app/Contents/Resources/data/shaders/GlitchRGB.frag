#version 150
#define AMPLITUDE 0.1
#define SPEED 0.1

uniform sampler2D tex;
uniform sampler2D tex2;
uniform vec2 dimensions;
uniform float time;
in vec2 coord;
out vec4 outputColor;

vec4 rgbShift( in vec2 p , in vec4 shift) {
  shift *= 2.0*shift.w - 1.0;
  vec2 rs = vec2(shift.x,-shift.y);
  vec2 gs = vec2(shift.y,-shift.z);
  vec2 bs = vec2(shift.z,-shift.x);
  
  vec4 r = texture(tex, p+rs, 0.0);
  vec4 g = texture(tex, p+gs, 0.0);
  vec4 b = texture(tex, p+bs, 0.0);
  
  return vec4(r.r,g.g,b.b, max(max(r.a, g.a), b.a));
}

vec4 noise( in vec2 p ) {
  return texture(tex2, p, 0.0);
}

vec4 vec4pow( in vec4 v, in float p ) {
  // Don't touch alpha (w), we use it to choose the direction of the shift
  // and we don't want it to go in one direction more often than the other
  return vec4(pow(v.x,p),pow(v.y,p),pow(v.z,p),v.w);
}

void main()
{
  vec2 p = coord.xy / dimensions.xy;
  vec4 t = texture(tex, p);

  vec4 c = vec4(0.0,0.0,0.0, 0.0);
  
  // Elevating shift values to some high power (between 8 and 16 looks good)
  // helps make the stuttering look more sudden
  vec4 shift = vec4pow(noise(vec2(SPEED*time,2.0*SPEED*time/25.0 )),8.0)
  *vec4(AMPLITUDE,AMPLITUDE,AMPLITUDE,1.0);;
  
  c += rgbShift(p, shift);
  if (c.a < 0.05) { outputColor = vec4(0.0); return; }

  outputColor = c;
}
