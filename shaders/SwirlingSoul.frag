#version 150

uniform sampler2D tex;
uniform vec2 dimensions;
uniform float time;
in vec2 coord;
out vec4 outputColor;


//inspired by new day of netgrind

#define COUNT 218.0

vec3 pal(in float t) {
    vec3 a =  vec3(0.184,0.776,0.737);// Updated color 1
    vec3 b = vec3(0.702,0.702,0.702); // Updated color 2
    vec3 c = vec3(0.788,0.188,0.910); // Updated color 3
    vec3 d = vec3(0.510,0.510,0.510);  // Updated color 4
  
 
    // Return the color by applying a cosine function to create smooth transitions between colors
    return a + b * cos(6.28318 * (c * t + d));
}

void main(  )
{
    float t = time*6.28*0.1;
    vec2 localCoord = coord - dimensions.xy * .5;
    // Normalized pixel coordinates (from 0 to 1)
    vec2 uv = localCoord/dimensions.xx * 15.;
    float d = length(uv);
    float f = 0.;
    float phase = t;
    float dir = 1.;
    float a = 0.;
    float len = -d*(cos(t) * .2 + .2);
    
    for(float i = 0.; i < COUNT; i+= 1.)
    {
    float p = phase + (sin(i + t)-1.)*.05+len;
        a = dot(normalize(uv), normalize(vec2(cos((p)*dir), sin((p)*dir))));
        a = max(0.,a);
        a = pow(a,20.);
        dir *= -1.;
        phase += mod(i , 6.28);
        
        f +=a;
        f = abs(mod(f + 1., 2.)-1.);
       //f =   fract(cos(f + 20. * sin(f + 10.))*0.01);
       
      f = cos(abs(f * 1.3));
        
    }
    
    float shine = 1.2;
    float scale = .5;
    f += shine - d * (scale+sin(t + dot(normalize(uv), vec2(1.,0.))*12.)*0.01);
    f = max(f, 0.);
    vec3 pink = vec3(0.984,0.514,0.984);
    
   localCoord += dimensions.xy * .5;

    vec2 uv0 = localCoord/dimensions.xx;

    vec3 palCol = pal(phase);
   //color

    vec3 c = mix(vec3(0.), pink, f);
   //webcam 
    c = min(max(c, 0.),1.);
    c = 1.0-vec3(.6, .4, .3)*3.*(1.0-c);

    // Output to screen
    outputColor = vec4(c,1.);
}

