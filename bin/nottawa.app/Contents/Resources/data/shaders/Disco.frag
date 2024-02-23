#version 150

#define SCALE_X 16.
#define SCALE_Y 5.
#define TIME_SCALE 0.1
#define BORDER 1.2

uniform sampler2D tex;
uniform vec2 dimensions;
uniform float time;
in vec2 coord;
out vec4 outputColor;

float hash1( float n ) { return fract(sin(n)*437518.56453)+0.1; }

float box(vec2 st) {
    vec2 s = vec2(0.001); // border size
    vec2 w = vec2(0.05); // border smoothness
    vec2 uv = smoothstep(s,s+w,st);
    uv *= smoothstep(s,s+w,vec2(1.0)-st);
    return uv.x*uv.y*(1.-length(0.5-st)*0.75);
}

// translate color from HSB space to RGB space
vec3 hsb2rgb( in vec3 c ){
    vec3 rgb = clamp(abs(mod(c.x*6.0+vec3(0.0,4.0,2.0),6.0)-3.0)-1.0,0.0,1.0 );
    rgb = rgb*rgb*(3.0-2.0*rgb);
    return c.z * mix(vec3(1.0), rgb, c.y);
}


float fbm (in vec2 st) {
  float value = (sin(time*5.*hash1(st.x+hash1(st.y)))+4.0)*0.25;
    return max(0.75,pow(value,3.));
}


void main()
{
 vec2 g = coord;
  vec2 si = dimensions.xy;
  vec2 uv = (g+g-si)/si.y;
  
  float a = atan(uv.x, uv.y);
  float r = length(uv);
  
  uv = vec2(a,r/dot(uv,uv));

    vec3 color = vec3(0.);
    float c = 1.;

    float lt = mod(time*TIME_SCALE,1./SCALE_Y); //local time cycle
    
    
    uv.x *= SCALE_X/3.1415926;
    float i = floor(uv.x); // row
    uv.x = fract(uv.x);

    if (uv.y < BORDER-lt)
    {
        uv.y += time*TIME_SCALE;
        uv.y *= SCALE_Y;
        float j = floor(uv.y);

        uv.y = fract(uv.y);
        color = hsb2rgb(vec3((hash1(j+hash1(i))),0.7,fbm(vec2(i,j))))*box(uv);
    }
    else {
        float fade = smoothstep (3.0,0.,sqrt(uv.y)); //fade to darkness
        uv.y += lt - BORDER;
        float j = floor ((BORDER-lt+time*TIME_SCALE)*SCALE_Y+floor(uv.y));
    uv.y = fract(uv.y)*SCALE_Y + min(0., 1.-SCALE_Y+lt*(SCALE_Y/TIME_SCALE*SCALE_Y*hash1(i+hash1(j))+1.));
      c *=  box (uv); //cell
        c += smoothstep(3.0,1.0,sqrt(uv.y))*smoothstep(1.0,1.05,uv.y)*smoothstep(0.5,0.45,abs(uv.x-0.5))*0.25; //cell trail
        c *= fade; //fade to darkness
        color = mix (color,hsb2rgb(vec3((hash1(j+hash1(i))),0.7,fbm(vec2(i,j)))),c);

    }
  outputColor = vec4(color,1.0);
}
