#version 120

#define MAX_SIZE 99

uniform sampler2DRect tex;
varying vec2 coord;
uniform vec2 dimensions;
uniform int size;
uniform float blur_mix;


// 16x acceleration of https://www.shadertoy.com/view/4tSyzy
// by applying gaussian at intermediate MIPmap level.

float normpdf(in float x, in float sigma)
{
  return 0.39894*exp(-0.5*x*x/(sigma*sigma))/sigma;
}


void main() {
  vec4 orig = texture2DRect(tex, coord);
  vec3 c = orig.rgb;
  
  //declare stuff
  int mSize = size;
  int kSize = (size-1)/2;
  float kernel[MAX_SIZE];
  vec3 final_colour = vec3(0.0);
  
  //create the 1-D kernel
  float sigma = 7.0;
  float Z = 0.0;
  for (int j = 0; j <= kSize; ++j)
  {
    kernel[kSize+j] = kernel[kSize-j] = normpdf(float(j), sigma);
  }
  
  //get the normalization factor (as the gaussian has been clamped)
  for (int j = 0; j < size; ++j)
  {
    Z += kernel[j];
  }
  
  //read out the texels
  for (int i=-kSize; i <= kSize; ++i)
  {
    for (int j=-kSize; j <= kSize; ++j)
    {
      final_colour += kernel[kSize+j]*kernel[kSize+i]*texture2DRect(tex, (coord+vec2(float(i), float(j)))).rgb;
    }
  }
  
  
  gl_FragColor = mix(orig, vec4(final_colour/(Z*Z), 1.0), blur_mix);
}
