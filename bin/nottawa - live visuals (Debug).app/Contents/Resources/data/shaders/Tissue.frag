#version 150

uniform vec2 dimensions;
uniform float time;
uniform float modulationSpeed;
uniform float distortionAmount;
uniform float waveIntensity;
uniform int cycles;

in vec2 coord;
out vec4 outputColor;

mat2 rotate2D(float r){
    return mat2(cos(r), sin(r), -sin(r), cos(r));
}

float noise(vec2 p) {
    return fract(sin(dot(p, vec2(12.9898, 78.233))) * 43758.5453);
}

void main()
{
  vec2 n = vec2(0.,0.);
  vec2 q = vec2(0.,0.);
  vec2 p = (coord.xy-.5*dimensions)/dimensions.y;
  float d = dot(p,p);
  mat2 m = rotate2D(waveIntensity);
  float S = 9.;
  float a = 0.;
  for(int j = 0; j < cycles; j++){
    p *= m;
    n *= m;
    q = p * S + time * 4. + sin(time * modulationSpeed - d) * .8 + j + n;
    a += dot(cos(q) / S, vec2(.2));
    n -= sin(q);
    S *= 1.2 * distortionAmount;
    outputColor = vec4(0., 0., 0., 1.);
  }
  outputColor += (a + .2) * vec4(4, 2, 1, 0.) + a + a - d;
}
