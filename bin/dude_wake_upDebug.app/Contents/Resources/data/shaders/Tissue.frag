#version 150

uniform vec2 dimensions;
uniform float time;
uniform float alpha;
uniform float beta;
uniform float gamma;

in vec2 coord;
out vec4 outputColor;

mat2 rotate2D(float r){
    return mat2(cos(r), sin(r), -sin(r), cos(r));
}

void main()
{
  vec2 n = vec2(0.,0.);
  vec2 q = vec2(0.,0.);
  vec2 p = (coord.xy-.5*dimensions)/dimensions.y;
  float d = dot(p,p);
  mat2 m = rotate2D(5.);
  int j = 0;
  float S = 9.;
  float a = 0.;
  for(int j = 0; j < 30; j++){
    p*=m;
    n*=m;
    q=p*S+time*4.+sin(time*alpha -d*beta)*.8+j+n;
    a+=dot(cos(q)/S,vec2(.2));
    n-=sin(q);
    S*=1.2;
    outputColor = vec4(0., 0., 0., 1.);
  }
  outputColor += (a+.2)*vec4(4,2,1,0.)+a+a-d;
}



