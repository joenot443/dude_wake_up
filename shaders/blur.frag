#version 150

// Add uniforms to select channels
uniform sampler2D tex;
uniform vec2 dimensions;
uniform float time;
uniform float radius;

in vec2 coord;
out vec4 outputColor;

// Can go down to 10 or so, and still be usable, probably...
#define ITERATIONS 30

// Set this to 0.0 to stop the pixel movement.
#define TIME time

#define TAU  6.28318530718

//-------------------------------------------------------------------------------------------
// Use last part of hash function to generate new random radius and angle...
vec2 Sample(inout vec2 r)
{
  r = fract(r * vec2(33.3983, 43.4427));
  return r-.5;
  //return sqrt(r.x+.001) * vec2(sin(r.y * TAU), cos(r.y * TAU))*.5; // <<=== circular sampling.
}

//-------------------------------------------------------------------------------------------
#define HASHSCALE 443.8975
vec2 Hash22(vec2 p)
{
  vec3 p3 = fract(vec3(p.xyx) * HASHSCALE);
  p3 += dot(p3, p3.yzx+19.19);
  return fract(vec2((p3.x + p3.y)*p3.z, (p3.x+p3.z)*p3.y));
}

//-------------------------------------------------------------------------------------------
vec4 Blur(vec2 uv, float radius)
{
  radius = radius * .04;
  
  vec2 circle = vec2(radius) * vec2((dimensions.y / dimensions.x), 1.0);
  
  // Remove the time reference to prevent random jittering if you don't like it.
  vec2 random = Hash22(uv);
  
  // Do the blur here...
  vec4 acc = vec4(0.0);
  for (int i = 0; i < ITERATIONS; i++)
  {
    acc += texture(tex, uv + circle * Sample(random), radius*10.0);
  }
  return acc / float(ITERATIONS);
}

//-------------------------------------------------------------------------------------------
void main(  )
{
  vec2 uv = coord.xy / dimensions.xy;
  
  float r = radius;
  vec4 result = Blur(uv * vec2(1.0, 1.0), r);
  if (result.r < 0.1 && result.g < 0.1 && result.b < 0.1) {
    outputColor = vec4(0.0);
    return;
  } 
  
  outputColor = result;
}
