#version 150

uniform sampler2D tex;
uniform vec2 dimensions;
uniform float time;
uniform float speed;
in vec2 coord;
out vec4 outputColor;


#define c(a) (sin(a)*.5+.5)
#define g    (time*.5*speed)

float b(vec3 p, vec3 s) { // box sdf
  vec3 q = abs(p) - s; return length(max(q,0.0)) + min(max(q.x,max(q.y,q.z)),0.);
}

void main() {
    float a = 3.;
    float r = 3.;
    float t = 3.;
    
    vec2 n = dimensions.xy;
    
    for (a = 0.; a < 150. && t > .002*r && r < 50.; a++) {
        vec3 p = normalize(vec3((coord - n * .5) / n.y * 1.4, 1.)) * r;
        p.z += g;
        
        float angle = mix(c(g), -c(g), c((g - 3.14) / 2.)) * r * .75;
        p.xy *= mat2(cos(angle), sin(angle), -sin(angle), cos(angle));
        
        float displacement = max(b(fract(p + .5) - .5, vec3(mix(.2, .45, c(g)))), -b(p, vec2(1.1, 1e9).xxy)) * .85;
        r += t = displacement;
    }
    
    vec3 color = cos(vec3(mix(2.05, 1.85, c(g)), 2.1, 2.15) * r - g) * exp(-r * .06);
    outputColor = vec4(color, 1.0);
}
