#version 150

uniform sampler2D tex;
uniform vec2 dimensions;
uniform float time;
uniform float alpha;
uniform float beta;
uniform float gamma;
in vec2 coord;
out vec4 outputColor;

#define hash3(p)  fract(sin(1e3*dot(p,vec3(1,57,-13.7)))*4375.5453)        // rand


// --- noise from procedural pseudo-Perlin (better but not so nice derivatives) ---------
// ( adapted from IQ )

float i_noise3( vec3 x ) {
  vec3 p = floor(x);
  vec3 f = fract(x);
  
  f = f*f*(3.-2.*f);  // or smoothstep     // to make derivative continuous at borders
  
  
  return mix( mix(mix( hash3(p+vec3(0,0,0)), hash3(p+vec3(1,0,0)),f.x),       // triilinear interp
                  mix( hash3(p+vec3(0,1,0)), hash3(p+vec3(1,1,0)),f.x),f.y),
             mix(mix( hash3(p+vec3(0,0,1)), hash3(p+vec3(1,0,1)),f.x),
                 mix( hash3(p+vec3(0,1,1)), hash3(p+vec3(1,1,1)),f.x),f.y), f.z);
}


#define noise(x) (i_noise3(x)+i_noise3(x+11.5)) / 2. // pseudoperlin improvement from foxes idea


void main() // ------------ draw isovalues
{
  vec2 R = dimensions.xy;
  float n = noise(vec3(coord*8./R.y, .1*time)),
  v = sin(6.28*10.*n),
  t = time;
  
  v = smoothstep(1.,0., .5*abs(v)/fwidth(v));
  
  outputColor = mix( exp(-33/R.y )
                    * texture( tex, (coord+vec2(1,sin(t)))/R), // .97
                    
                    alpha+.5*sin(12.*n+vec4(0,gamma,beta,0)),
                    v );
  
}
