#version 150

uniform sampler2D tex;
uniform vec2 dimensions;
uniform float time;
uniform float gamma;
uniform float pixelSize;
uniform float ditherAmount;
uniform int palette;
uniform float hueShift;
uniform float saturation;
in vec2 coord;
out vec4 outputColor;

// color brightnesses (ZX Spectrum)
#define CFULL 1.0
#define CHALF 0.8431372549

// ===== ZX Spectrum quantization helpers =====

vec4 smap(vec3 c)
{
  c=pow(c,vec3(gamma));
  if( (c.r>CHALF) || (c.g>CHALF) || (c.b>CHALF) )
  {
    return vec4( c.rgb , 1.0);
  }
  else
  {
    return vec4( min( (c.rgb/CHALF) , vec3(1.0,1.0,1.0) ) , 0.0 );
  }
}

vec4 bmap(vec3 c)
{
  c=pow(c,vec3(gamma));
  if( (c.r>CHALF) || (c.g>CHALF) || (c.b>CHALF) )
  {
    return vec4( floor(c.rgb+vec3(0.5)) , 1.0);
  }
  else
  {
    return vec4( min( floor((c.rgb/CHALF)+vec3(0.5)) , vec3(1.0,1.0,1.0) ) , 0.0 );
  }
}

vec3 fmap(vec4 c)
{
  if(c.a>=0.5)
  {
    return c.rgb*vec3(CFULL,CFULL,CFULL);
  }
  else
  {
    return c.rgb*vec3(CHALF,CHALF,CHALF);
  }
}

// ===== Color adjustments =====

vec3 rgb2hsv(vec3 c)
{
  vec4 K = vec4(0.0, -1.0/3.0, 2.0/3.0, -1.0);
  vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
  vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));
  float d = q.x - min(q.w, q.y);
  float e = 1.0e-10;
  return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
}

vec3 hsv2rgb(vec3 c)
{
  vec4 K = vec4(1.0, 2.0/3.0, 1.0/3.0, 3.0);
  vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
  return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

vec3 adjustColor(vec3 c)
{
  if (abs(hueShift) < 0.0001 && abs(saturation - 1.0) < 0.0001) return c;
  vec3 hsv = rgb2hsv(c);
  hsv.x = fract(hsv.x + hueShift);
  hsv.y = clamp(hsv.y * saturation, 0.0, 1.0);
  return hsv2rgb(hsv);
}

// ===== Custom palette tables =====

// Palette sizes
int paletteSize(int p)
{
  if (p == 1) return 4;   // CGA 0
  if (p == 2) return 4;   // CGA 1
  if (p == 3) return 4;   // Game Boy
  if (p == 4) return 16;  // C64
  if (p == 5) return 4;   // Mono Green
  if (p == 6) return 4;   // Mono Amber
  if (p == 7) return 4;   // Mono White
  return 1;
}

vec3 paletteColor(int p, int i)
{
  // CGA palette 0 (high-intensity Cyan/Magenta/White)
  if (p == 1) {
    if (i == 0) return vec3(0.0, 0.0, 0.0);
    if (i == 1) return vec3(0.333, 1.0, 1.0);
    if (i == 2) return vec3(1.0, 0.333, 1.0);
    return vec3(1.0, 1.0, 1.0);
  }
  // CGA palette 1 (high-intensity Green/Red/Yellow)
  if (p == 2) {
    if (i == 0) return vec3(0.0, 0.0, 0.0);
    if (i == 1) return vec3(0.333, 1.0, 0.333);
    if (i == 2) return vec3(1.0, 0.333, 0.333);
    return vec3(1.0, 1.0, 0.333);
  }
  // Game Boy DMG (4 olive greens)
  if (p == 3) {
    if (i == 0) return vec3(0.058, 0.219, 0.058);
    if (i == 1) return vec3(0.188, 0.384, 0.188);
    if (i == 2) return vec3(0.545, 0.674, 0.058);
    return vec3(0.607, 0.737, 0.058);
  }
  // Commodore 64 (16 colors)
  if (p == 4) {
    if (i == 0)  return vec3(0.000, 0.000, 0.000);
    if (i == 1)  return vec3(1.000, 1.000, 1.000);
    if (i == 2)  return vec3(0.541, 0.227, 0.181);
    if (i == 3)  return vec3(0.439, 0.713, 0.741);
    if (i == 4)  return vec3(0.553, 0.255, 0.624);
    if (i == 5)  return vec3(0.357, 0.624, 0.286);
    if (i == 6)  return vec3(0.247, 0.184, 0.580);
    if (i == 7)  return vec3(0.788, 0.819, 0.498);
    if (i == 8)  return vec3(0.561, 0.341, 0.176);
    if (i == 9)  return vec3(0.380, 0.255, 0.000);
    if (i == 10) return vec3(0.737, 0.396, 0.353);
    if (i == 11) return vec3(0.318, 0.318, 0.318);
    if (i == 12) return vec3(0.510, 0.510, 0.510);
    if (i == 13) return vec3(0.612, 0.866, 0.553);
    if (i == 14) return vec3(0.486, 0.435, 0.831);
    return vec3(0.690, 0.690, 0.690);
  }
  // Mono Green (CRT)
  if (p == 5) {
    if (i == 0) return vec3(0.0, 0.0, 0.0);
    if (i == 1) return vec3(0.0, 0.294, 0.0);
    if (i == 2) return vec3(0.0, 0.588, 0.0);
    return vec3(0.196, 1.0, 0.196);
  }
  // Mono Amber (CRT)
  if (p == 6) {
    if (i == 0) return vec3(0.0, 0.0, 0.0);
    if (i == 1) return vec3(0.294, 0.196, 0.0);
    if (i == 2) return vec3(0.706, 0.470, 0.0);
    return vec3(1.0, 0.706, 0.0);
  }
  // Mono White
  if (p == 7) {
    if (i == 0) return vec3(0.0);
    if (i == 1) return vec3(0.333);
    if (i == 2) return vec3(0.666);
    return vec3(1.0);
  }
  return vec3(0.0);
}

// 4x4 Bayer dither matrix (values normalized -0.5..0.5)
float bayer4(vec2 pv)
{
  int x = int(mod(pv.x, 4.0));
  int y = int(mod(pv.y, 4.0));
  int i = x + y * 4;
  // standard 4x4 ordered dither matrix / 16 - 0.5
  float m;
  if (i == 0) m = 0.0;        else if (i == 1) m = 8.0;
  else if (i == 2) m = 2.0;   else if (i == 3) m = 10.0;
  else if (i == 4) m = 12.0;  else if (i == 5) m = 4.0;
  else if (i == 6) m = 14.0;  else if (i == 7) m = 6.0;
  else if (i == 8) m = 3.0;   else if (i == 9) m = 11.0;
  else if (i == 10) m = 1.0;  else if (i == 11) m = 9.0;
  else if (i == 12) m = 15.0; else if (i == 13) m = 7.0;
  else if (i == 14) m = 13.0; else m = 5.0;
  return (m / 16.0) - 0.5;
}

vec3 quantizeToPalette(vec3 c, int p, vec2 pv)
{
  c = pow(clamp(c, 0.0, 1.0), vec3(gamma));
  c = adjustColor(c);
  c += vec3(bayer4(pv) * ditherAmount * 0.5);

  int n = paletteSize(p);
  float bestDist = 1.0e9;
  vec3 bestColor = vec3(0.0);
  for (int i = 0; i < 16; i++) {
    if (i >= n) break;
    vec3 pc = paletteColor(p, i);
    vec3 diff = c - pc;
    float d = dot(diff, diff);
    if (d < bestDist) { bestDist = d; bestColor = pc; }
  }
  return bestColor;
}

// ===== ZX Spectrum 8x8 attribute clash (original behavior) =====

void zxspectrum_clash(out vec4 outputColor, in vec2 coord)
{
  vec2 pv = floor(coord.xy/pixelSize);
  vec2 bv = floor(pv/8.0)*8.0;
  vec2 sv = floor(dimensions.xy/pixelSize);

  vec4 min_cs=vec4(1.0,1.0,1.0,1.0);
  vec4 max_cs=vec4(0.0,0.0,0.0,0.0);
  float bright=0.0;

  for(int py=1;py<8;py++)
  {
    for(int px=0;px<8;px++)
    {
      vec3 src = adjustColor(texture(tex,(bv+vec2(px,py))/sv).rgb);
      vec4 cs=bmap(src);
      bright+=cs.a;
      min_cs=min(min_cs,cs);
      max_cs=max(max_cs,cs);
    }
  }

  vec4 c;

  if(bright>=24.0) { bright=1.0; } else { bright=0.0; }

  if( max_cs.rgb==min_cs.rgb )
  {
    min_cs.rgb=vec3(0.0,0.0,0.0);
  }

  if( max_cs.rgb==vec3(0.0,0.0,0.0) )
  {
    bright=0.0;
    max_cs.rgb=vec3(0.0,0.0,1.0);
    min_cs.rgb=vec3(0.0,0.0,0.0);
  }

  vec3 c1=fmap(vec4(max_cs.rgb,bright));
  vec3 c2=fmap(vec4(min_cs.rgb,bright));

  vec3 cs=adjustColor(texture(tex,pv/sv).rgb);

  vec3 d= (cs+cs) - (c1+c2) ;
  float dd=d.r+d.g+d.b;

  if( mod(pv.x+pv.y,2.0)==1.0)
  {
    outputColor=vec4(
                     dd>=-(ditherAmount*0.5) ? c1.r : c2.r,
                     dd>=-(ditherAmount*0.5) ? c1.g : c2.g,
                     dd>=-(ditherAmount*0.5) ? c1.b : c2.b,
                     1.0);
  }
  else
  {
    outputColor=vec4(
                     dd>=(ditherAmount*0.5) ? c1.r : c2.r,
                     dd>=(ditherAmount*0.5) ? c1.g : c2.g,
                     dd>=(ditherAmount*0.5) ? c1.b : c2.b,
                     1.0);
  }
}

// ===== Custom palette path =====

void custom_palette(out vec4 outputColor, in vec2 coord)
{
  vec2 pv = floor(coord.xy/pixelSize);
  vec2 sv = floor(dimensions.xy/pixelSize);
  vec3 c = texture(tex, pv/sv).rgb;
  outputColor = vec4(quantizeToPalette(c, palette, pv), 1.0);
}

void main()
{
  if (palette == 0) {
    zxspectrum_clash(outputColor, coord);
  } else {
    custom_palette(outputColor, coord);
  }
}
