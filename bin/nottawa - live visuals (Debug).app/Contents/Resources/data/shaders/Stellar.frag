#version 150

uniform sampler2D tex;
uniform vec2 dimensions;
uniform float time;
in vec2 coord;
out vec4 outputColor;

// Radial blur samples. More is always better, but there's frame rate to consider.
const float SAMPLES = 40.; 


// 2x1 hash. Used to jitter the samples.
float hash( vec2 p ){ return fract(sin(dot(p, vec2(41, 289)))*45758.5453); }


vec3 lOff(){    
    return vec3(0., 0., 0.); // x and y (z is not important)
}

	

///////////////////////////////////
////     Rainbow functions     ////
///////////////////////////////////

vec3 rainbow_1(float x) {    
    return vec3(
        (tanh(-10.*x + 2.) + 1.) * 0.5 + (tanh(10.*x - 7.) + 1.) * 0.5,
        (tanh(-10.*x + 3.) + 1.) * 0.5,
        (tanh(-10.*x + 6.) + 1.) * 0.5
    );
}

vec3 rainbow_2(float x) {
    
    float r_w = 1.;
    float g_w = (tanh(-10.*x + 6.) + 1.) * 0.5;
    float b_w = (tanh(-10.*x + 3.) + 1.) * 0.5;
    
    return vec3(r_w, g_w, b_w);   
}


///////////////////////////////////
///////////////////////////////////


void main(  ){
    vec2 uv = coord.xy / dimensions.xy;

    
    // Radial blur factors.

    // Falloff, as we radiate outwards.
    float decay = 1.; 
    // Controls the sample density, which in turn, controls the sample spread.
    float density = 0.9; 
    // Sample weight. Decays as we radiate outwards.
    float weight = 0.05; 
    
    // Light offset. Kind of fake. See above.
    vec3 l = lOff();
    
    // Offset texture position (uv - .5), offset again by the fake light movement.
    // It's used to set the blur direction (a direction vector of sorts), and is used 
    // later to center the spotlight.
    //
    // The range is centered on zero, which allows the accumulation to spread out in
    // all directions. Ie; It's radial.
    vec2 tuv =  uv - .5 - l.xy*.45;
    
    // Dividing the direction vector above by the sample number and a density factor
    // which controls how far the blur spreads out. Higher density means a greater 
    // blur radius.
    vec2 dTuv = tuv*density/SAMPLES;
    
    // Grabbing a portion of the initial texture sample. Higher numbers will make the
    // scene a little clearer, but I'm going for a bit of abstraction.
    vec4 col = texture(tex, uv.xy)*0.8;
    
    // Jittering, to get rid of banding. Vitally important when accumulating discontinuous 
    // samples, especially when only a few layers are being used.
    uv += dTuv*(hash(uv.xy)*2. - 1.);
    
    // The radial blur loop. Take a texture sample, move a little in the direction of
    // the radial direction vector (dTuv) then take another, slightly less weighted,
    // sample, add it to the total, then repeat the process until done.
    
    vec4 currentColor;
    float threshold = 0.7;
    
    for(float i=0.; i < SAMPLES; i++){
        uv -= dTuv;
        currentColor = texture(tex, uv);
        col.rgb += (
            step(threshold, length(currentColor.rgb)/sqrt(3.)) *
            currentColor.rgb * 
            rainbow_2(i/SAMPLES) * 
            weight
        );
        weight *= decay;
    }
    
    // Centrilized brightness
    // col *= (1. - dot(tuv, tuv)*.75);
    
    // Smoothstepping the final color, just to bring it out a bit
    outputColor = sqrt(smoothstep(0., 1., col));
}

