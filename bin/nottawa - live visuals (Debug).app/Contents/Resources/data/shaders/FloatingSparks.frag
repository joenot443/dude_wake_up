#version 150

uniform sampler2D tex;
uniform vec2 dimensions;
uniform float time;
in vec2 coord;
out vec4 outputColor;

#define n_tau 6.2831

float f_n_rnd11(float p)
{
    p = fract(p * .1031);
    p *= p + 33.33;
    p *= p + p;
    return fract(p);
}

void main(  )
{
    // Normalized pixel coordinates (from 0 to 1)
    vec2 o_fc = (coord.xy-dimensions.xy*.5)/dimensions.y;

    vec3 o_clr_sum = vec3(0.);
    vec3 o_clr = vec3(0.);
    vec3[] a_o_clr = vec3[] (
        vec3(0.086, 0.208, 0.816),
        vec3(0.243, 0.639, 0.435),
        vec3(0.565, 0.282, 0.796), 
        vec3(0.863, 0.878, 0.102), 
        vec3(0.655, 0.216, 0.184)
    );
    float n_len_a_o_clr = float(a_o_clr.length());

    float n_its = 100.;
    float n_dmin = 1.;
    vec3 o_clr_min = vec3(0.);
    float n_dprod = 1.;
    for(float n_it = 0.; n_it < n_its; n_it+=1.){
        float n_it_nor = n_it/n_its;
        float n_radius = (sin(time+n_it_nor*n_tau*2324.2)*.5+.5)*.5;
        float n_radians = n_tau * n_it_nor;
   
        float n_idx = f_n_rnd11(n_it_nor);
        // float n_idx = fract(n_it_nor);
        vec3 o_clr = a_o_clr[int(n_idx*n_len_a_o_clr)];
        vec2 o_p = vec2(
            sin(n_radians) * n_radius,
            cos(n_radians) * n_radius
        );
        float n_d = length(o_p-o_fc);
        // n_d = smoothstep(0.1, 0., n_d);
        n_d = pow(n_d, 1./5.);
        o_clr_sum += o_clr*smoothstep(n_idx, 0., n_d);
        // o_clr = vec3(n_d);
        if(n_d < n_dmin){
            n_dmin = n_d;
            o_clr_min = o_clr;
        }
        n_dprod*= n_d;
    }
    outputColor = vec4(0.);
    float n_d = 1.-pow(n_dmin, 1./3.);
    n_d = smoothstep(.3, 1., n_d);
    // outputColor = vec4(n_d);
    // outputColor = vec4(vec3(n_d*o_clr_min), 1.);
    outputColor = vec4(o_clr_sum*pow(n_dprod,1./22.)*4., 1.);
}