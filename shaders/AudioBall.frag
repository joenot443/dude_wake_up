#version 150

uniform sampler2D tex;
uniform vec2 dimensions;
uniform float time;
uniform float dotCount;
uniform float ringRadius;
uniform float dotBrightness;
in vec2 coord;
uniform float audio[256];
out vec4 outputColor;

//convert HSV to RGB
vec3 hsv2rgb(vec3 c){
  vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
  vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
  return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

void main(  ) {

  vec2 p=(coord.xy-.5*dimensions.xy)/min(dimensions.x,dimensions.y);
  vec3 c=vec3(0,0,0.1); //background color

  for(float i=0.;i<dotCount; i++){

    //read frequency for this dot from audio input channel
    //based on its index in the circle
    float vol = audio[int(i/dotCount * 256.)];
    float b = vol * dotBrightness;

    //get location of dot
    float x = ringRadius*cos(2.*3.14*float(i)/dotCount);
    float y = ringRadius*sin(2.*3.14*float(i)/dotCount);
    vec2 o = vec2(x,y);

    //get color of dot based on its index in the
    //circle + time to rotate colors
    vec3 dotCol = hsv2rgb(vec3((i + time*10.)/dotCount,1.,1.0));

    //get brightness of this pixel based on distance to dot
    c += b/(length(p-o))*dotCol;
  }

  outputColor = vec4(c,1);
}
