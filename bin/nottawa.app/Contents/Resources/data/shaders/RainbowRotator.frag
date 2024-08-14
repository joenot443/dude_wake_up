#version 150

uniform sampler2D tex;
uniform vec2 dimensions;
uniform float time;
uniform float frequency;
uniform float height;
in vec2 coord;
out vec4 outputColor;


#define twopi 6.28319
#define SMOOTH_HSV;

//HSV functions from iq (https://www.shadertoy.com/view/MsS3Wc)
#ifdef SMOOTH_HSV
vec3 hsv2rgb( in vec3 c )
{
  vec3 rgb = clamp( abs(mod(c.x*6.0+vec3(0.0,4.0,2.0),6.0)-3.0)-1.0, 0.0, 1.0 );
  
  rgb = rgb*rgb*(3.0-2.0*rgb); // cubic smoothing
  
  return c.z * mix( vec3(1.0), rgb, c.y);
}
#else
vec3 hsv2rgb( in vec3 c )
{
  vec3 rgb = clamp( abs(mod(c.x*6.0+vec3(0.0,4.0,2.0),6.0)-3.0)-1.0, 0.0, 1.0 );
  
  return c.z * mix( vec3(1.0), rgb, c.y);
}
#endif

//From Sam Hocevar: http://lolengine.net/blog/2013/07/27/rgb-to-hsv-in-glsl
vec3 rgb2hsv(vec3 c)
{
  vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
  vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
  vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));
  
  float d = q.x - min(q.w, q.y);
  float e = 1.0e-10;
  return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
}

void main()
{
  vec2 uv = coord;
  vec3 ccol = texture(tex, uv).rgb;
  float hued = mod(time*frequency, 1.) + uv.y*height;
  vec3 hsv = rgb2hsv(ccol);
  ccol = hsv2rgb(vec3(hsv.x+hued, hsv.y, hsv.z));
  
  outputColor = vec4(ccol, 1.);
}
