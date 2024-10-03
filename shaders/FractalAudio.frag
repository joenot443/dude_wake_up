#version 150

uniform vec2 dimensions;
uniform float time;
uniform float audio[256];
in vec2 coord;
out vec4 outputColor;

// Fork of "Fractal 77_gaz" by gaz. https://shadertoy.com/view/fdy3WG
// 2024-02-18 19:18:54

#define R(p,a,r)mix(a*dot(p,a),p,cos(r))+sin(r)*cross(p,a)
#define H(h)(cos((h)*6.3+vec3(0,23,21))*.5+.5)
vec3 pow3(vec3 x, int y) {
  while (y>0) {
    x*=x;
    y=y-1;
  }
  return x;
}

float fft(float x) {
  // Fixed length of the audio array
  const int audioLength = 256;
  
  // Scale x to fit the range of the array (0 to audioLength - 1)
  float scaledX = x * float(audioLength - 1);
  
  // Get the integer part of scaledX
  int i = int(scaledX);
  
  // Ensure i is within bounds
  if (i < 0 || i >= audioLength - 1) {
    return 0.0; // Return a default value for out-of-bounds
  }
  
  // Calculate the fractional part
  float frac = scaledX - float(i);
  
  // Access audio data directly from the uniform array
  float audioValue1 = audio[i];
  float audioValue2 = audio[i + 1];
  
  // Perform linear interpolation between audio[i] and audio[i + 1]
  return mix(audioValue1, audioValue2, frac);
}
void main()
{
  outputColor=vec4(0);
  
  float bass  = audio[0];
  bass = clamp((bass-0.5)*2.0,0.0,1.0);
  
  vec3 p=vec3(0);
  vec2 r=dimensions;
  vec3 d=normalize(vec3((bass*2.+coord-.5*r.xy)/r.y,1));
  
  for(
      float i=0.,g=0.,e,s;
      ++i<99.;
      outputColor.rgb+=mix(vec3(1) ,H(log(s)),.7)*.08*exp(-i*i*e))
  {
    
    p=g*d;
    p.z-=.6;
    p=R(p,normalize(vec3(1,2,3)),time*.3);
    s=4.;
    for(int j=0;j++<8;) {
      p=abs(p),p=p.x<p.y?p.zxy:p.zyx,
      s*=e=1.8/min(dot(p,p),1.3),
      p=p*e-vec3(12,3,3)+2.*fft((e)/99.);
    }
    g+=e=length(p.xz)/s;
    
  }
  outputColor=vec4(pow3(outputColor.rgb,2),1.);
}
