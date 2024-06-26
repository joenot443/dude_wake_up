#version 150

uniform sampler2D tex;
uniform vec2 dimensions;
uniform float time;
in vec2 coord;
out vec4 outputColor;

// change to iChannel1 and use the cam, have fun
#define Channel tex

// normal noise with periodic flare for the tv noise stripps
float rand(vec2 co){ return sin(fract(fract(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453)); }


void main(  )
{
  // Normalized pixel coordinates (from 0 to 1)
  vec2 uv = coord/dimensions.xy;
  // shifting this noise up for better tv effect
  vec2 noiseuv = (coord * 2.0 - dimensions.xy + dimensions.y*3.0) / dimensions.y ;
  // sampling the noise value with the noise uv
  float value =  rand(noiseuv * (time+1.));
  // getting the pixel color from ichannel0 (camera in this case)
  vec4 pixel = texture(Channel, uv);
  // brightness formula
  float brightness =  (0.21 * pixel.r) + (0.72 * pixel.g) + (0.07 * pixel.b);
  
  outputColor = vec4(vec3(value * brightness), 1.0);
}
