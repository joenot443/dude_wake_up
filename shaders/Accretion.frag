#version 150

uniform sampler2D tex;
uniform vec2 dimensions;
uniform float time;
in vec2 coord;
out vec4 outputColor;

/*
 "Accretion" by @XorDev

 I discovered an interesting refraction effect
 by adding the raymarch iterator to the turbulence!
 https://x.com/XorDev/status/1936884244128661986
 */

void main()
{
  //Raymarch depth
  float z = 0;
  //Step distance
  float d = 0;
  //Raymarch iterator
  float i = 0;
  outputColor = vec4(1.0);
  //Clear outputColor and raymarch 100 steps
  for(outputColor*=i; i++<2e1; )
  {
    //Sample point (from ray direction)
    vec3 p = z*normalize(vec3(coord+coord,0)-dimensions.xyx)+.1;

    //Polar coordinates and additional transformations
    p = vec3(atan(p.y/.2,p.x)*2., p.z/3., length(p.xy)-5.-z*.2);

    //Apply turbulence and refraction effect
    for(d=0.; d++<7.;)
      p += sin(p.yzx*d+time+.3*i)/d;

    //Distance to cylinder and waves with refraction
    z += d = length(vec4(.4*cos(p)-.4, p.z));

    //Coloring and brightness
    outputColor += (1.+cos(p.x+i*.4+z+vec4(6,1,2,0)))/d;
  }
  //Tanh tonemap
  outputColor = tanh(outputColor*outputColor/4e2);
}
