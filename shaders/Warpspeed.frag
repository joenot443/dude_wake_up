#version 150

// https://www.shadertoy.com/view/4Xc3zH

uniform sampler2D tex;
uniform vec2 dimensions;
uniform float time;
uniform float speed;
in vec2 coord;
out vec4 outputColor;


#define pi 3.14159
#define oz vec2(1,0)

// Random normalized vector
vec2 randVec(vec2 p) {
  float r = fract(sin(dot(p, vec2(192.345, 741.85)))*4563.12);
  r *= 2.0*pi;
  r += time; // Rotate gradient vector to simulate 3d noise
  return vec2(cos(r), sin(r));
}

// Seamless tiled perlin noise
float perlin(vec2 p, vec2 t) {
  vec2 f = fract(p);
  vec2 s = smoothstep(0.0, 1.0, f);
  vec2 i = floor(p);
  // Apply mod() to vertex position to make it tileable
  float a = dot(randVec(mod(i,t)), f);
  float b = dot(randVec(mod(i+oz.xx,t)), f-oz.xy);
  float c = dot(randVec(mod(i+oz.yx,t)), f-oz.yx);
  float d = dot(randVec(mod(i+oz.xx,t)), f-oz.xx);
  return mix(mix(a, b, s.x), mix(c, d, s.x), s.y);
}

// Seamless tiled fractal noise
float fbm(vec2 p, vec2 t) {
  float a = 0.5;
  float r = 0.0;
  for (int i = 0; i < 8; i++) {
    r += a*perlin(p, t);
    a -= 0.5;
    p *= 2.0;
    t *= 2.0;
  }
  return r;
}

void main() {
  vec2 uv = coord/dimensions.y - dimensions.xy/dimensions.y/2.0;
  // Modified polar coordinates
  vec2 cuv = vec2((atan(uv.x, uv.y)+pi)/(2.0*pi), 0.005/length(uv)+0.01*speed*time);
  // Highlight at the center of the light source
  float hl = (1.0-length(uv));
  hl -= hl * hl;
  outputColor = vec4(pow(.1+0.5*fbm(10.0*cuv, vec2(20)), 10.0)+hl);
}
