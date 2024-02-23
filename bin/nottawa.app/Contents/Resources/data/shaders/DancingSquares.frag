#version 150
#define SIZE 6.0

uniform sampler2D tex;
uniform vec2 dimensions;
uniform float audio[256];
uniform float time;
in vec2 coord;
out vec4 outputColor;


void main(  )
{
    vec2 uv = ( coord -.5*  dimensions.xy ) / dimensions.x;
    uv *= mat2(1,1,-1,1)/sqrt(2.);
    uv += .5;
    uv *= 10.;
    
    vec2 rep = fract(uv);
         rep = min(rep,1.-rep);
    float value = fract ( 4.* min(rep.x,rep.y) ),
     squareDist = length( floor(uv) -4.5 ),
          edge = 2.* fract( ( time - squareDist*.5 ) / 4.) - 1.;
 
    value = edge < 0. ? value : 1.-value;
    value =   smoothstep( -.05, 0., .95*value - edge*edge)
            + squareDist*.1;
    
    outputColor = mix( vec4(1),vec4(.5,.75,1,1), value);
}
