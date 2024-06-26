#version 150

uniform sampler2D tex;
uniform vec2 dimensions;
uniform float time;
uniform float audio[256];
in vec2 coord;
out vec4 outputColor;


/////////////////////////////////////////////////////////////////////////////////
// Modified from relampago2048's "Fractal Audio 01"
// https://www.shadertoy.com/view/llB3W1
/////////////////////////////////////////////////////////////////////////////////

const int iters = 150;

int fractal(vec2 p, vec2 point) {
  vec2 so = (-1.0 + 2.0 * point) * 0.3;
  vec2 seed = vec2(0.098386255 + so.x, 0.6387662 + so.y);
  
  for (int i = 0; i < iters; i++) {
    
    if (length(p) > 2.0) {
      return i;
    }
    vec2 r = p;
    p = vec2(p.x * p.x - p.y * p.y, 2.0* p.x * p.y);
    p = vec2(p.x * r.x - p.y * r.y + seed.x, r.x * p.y + p.x * r.y + seed.y);
  }
  
  return 0;
}

vec3 color(int i) {
  float f = float(i)/float(iters) * 2.0;
  f=f*f*2.;
  return vec3((sin(f*2.0)), (sin(f*3.0)), abs(sin(f*7.0)));
}


float sampleMusicA() {
  return 0.6 * (
                audio[int(0.15*256.0)] +
                audio[int(0.3*256.0)]
                );
}

mat2 rotateX(float rad) {
  float c = cos(rad);
  float s = sin(rad);
  return mat2(
              c, -s,
              s, c
              );
}

vec4 fractalImg(vec2 uv, vec2 coord) {
  vec2 mouse = vec2(0.5);
  
  vec2 position = 3. * (-0.5 + coord.xy / dimensions.xy ) + vec2(0.2 * abs(cos(time * 0.21313)) * sin(sin(time)+cos(time)*.242), 0.1 * abs(cos(time * 0.1323)));
  position.x *= dimensions.x/dimensions.y;
  position *= rotateX(mod(time * .54, 6.3) + abs(.2 * sin(time / 5.3)));
  
  vec2 iFC = vec2(dimensions.x-coord.x,dimensions.y-coord.y);
  vec2 pos2 = 2. * (-0.5 + iFC.xy / dimensions.xy);
  pos2.x*=dimensions.x/dimensions.y;
  pos2 *= rotateX(mod(time * -.4235, 6.3) + abs(.2 * sin(time / 5.3)));
  
  vec4 t3 = texture(tex, vec2(length(position)/14.0,0.1) );
  float pulse = .2+sampleMusicA()*1.8;
  
  vec3 invFract = color(fractal(pos2,vec2(0.55+sin(time/18.+0.5)/2.0,pulse*.9)));
  
  vec3 fract4 = color(fractal(position/1.6,vec2(0.6+cos(time/20.+0.5)/2.0,pulse*.8)));
  
  vec3 c = color(fractal(position,vec2(10.1+sin(time/3.)/12.0,pulse)));
  
  t3=abs(vec4(0.5,0.1,0.5,1.)-t3)*.8;
  
  vec4 fract01 =  vec4( c , 1.0 );
  vec4 salida;
  salida = fract01 / t3 + fract01 * t3 + vec4(invFract,0.6) + vec4(fract4,0.25);
  return mix(salida * 2., vec4(audio[int(256.0 * uv.x)]), 0.3);
}

/////////////////////////////////////////////////////////////////////////////////
// Modified from t3hk0d3's "webcam roberts"
// https://www.shadertoy.com/view/MssGRB
/////////////////////////////////////////////////////////////////////////////////


const float sensitivity = 1.0 / 100.0;

float bw(vec2 coords) {
  vec4 lm = texture(tex, coords) * vec4(0.21, 0.71, 0.07, 1);
  
  return lm.r+lm.g+lm.b;
}

float outline(vec2 uv)
{
  vec2 of = vec2(sensitivity * pow(sampleMusicA(), 2.), 0);
  
  return sqrt(
              pow(abs(bw(uv) - bw(uv+of.xx)), 2.0) +
              pow(abs(bw(uv + of.xy) - bw(uv + of.yx)), 2.0)
              );
}

/////////////////////////////////////////////////////////////////////////////////
// Modified from  unycone's "Camera Distortion 01"
// https://www.shadertoy.com/view/XlXfRs
/////////////////////////////////////////////////////////////////////////////////

vec2 distort(vec2 uv)
{
  bool autoScale = true;
  float strength = sampleMusicA() * sign(sin(time / 5.)) * 0.25;
  float zoom = (autoScale ? abs(strength) + 1.0 : 1.0);
  
  
  
  vec2 p = (uv - 0.5) * 2.0; // map [0, 1] to [-1, 1]
  float theta = atan(p.y, p.x);
  float rd = length(p);
  float ru = rd * (1.0 + strength * rd * rd) / zoom;
  return vec2(cos(theta), sin(theta)) * ru / 2.0 + 0.5;
}


/////////////////////////////////////////////////////////////////////////////////
void main()
{
  vec2 uv = coord.xy / dimensions.xy;
  uv.x = 1.0 - uv.x;
  
  vec4 imgColor = texture(tex, uv);
//  uv = distort(uv); // Comment this line out to remove distortion
  
  float bwColor = outline(uv) * 2.;
  bwColor = min(round(bwColor * 8.), 2.) * bwColor;
  
  
  outputColor = imgColor + bwColor * (fractalImg(uv, coord) + fractalImg(-uv, coord));
  outputColor.a = 1.0;
}
