#version 150

#define MAX_SIZE 99

uniform sampler2D tex;
in vec2 coord;
uniform vec2 dimensions;
uniform float size;
out vec4 outputColor;

const int samples = 35,
          LOD = 2,         // gaussian done on MIPmap at scale LOD
          sLOD = 1 << LOD; // tile size = 2^LOD

float gaussian(vec2 i) {
    float sigma = samples * .25 * size;
    return exp( -.5* dot(i/=sigma,i) ) / ( 6.28 * sigma*sigma );
}

vec4 blur(sampler2D sp, vec2 U, vec2 scale) {
    vec4 O = vec4(0);
    int s = samples/sLOD;
    
    for ( int i = 0; i < s*s; i++ ) {
        vec2 d = vec2(i%s, i/s)*float(sLOD) - float(samples)/2.;
        O += gaussian(d) * texture( sp, U + scale * d , float(LOD) );
    }
    
    return O / O.a;
}

void main() {
    outputColor = blur(tex, coord/ dimensions.xy, 1./dimensions );
}
