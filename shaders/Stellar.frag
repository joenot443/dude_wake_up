#version 150

uniform sampler2D tex;
uniform vec2 dimensions;
uniform float time;
uniform float threshold;
uniform float decay;
uniform float density;
uniform float weight;
uniform vec2 center;
in vec2 coord;
out vec4 outputColor;

// Radial blur samples. More is always better, but there's frame rate to consider.
const float SAMPLES = 40.;


// 2x1 hash. Used to jitter the samples.
float hash( vec2 p ){ return fract(sin(dot(p, vec2(41, 289)))*45758.5453); }


///////////////////////////////////
////     Rainbow functions     ////
///////////////////////////////////

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

    // Sample weight. Decays as we radiate outwards.
    float w = weight;

    // Offset texture position relative to center.
    vec2 tuv = uv - center;

    // Dividing the direction vector by the sample number and a density factor
    // which controls how far the blur spreads out.
    vec2 dTuv = tuv * density / SAMPLES;

    // Grabbing a portion of the initial texture sample.
    vec4 col = texture(tex, uv.xy) * 0.8;

    // Jittering, to get rid of banding.
    uv += dTuv * (hash(uv.xy) * 2. - 1.);

    // The radial blur loop.
    vec4 currentColor;

    for(float i=0.; i < SAMPLES; i++){
        uv -= dTuv;
        currentColor = texture(tex, uv);
        col.rgb += (
            step(threshold, length(currentColor.rgb)/sqrt(3.)) *
            currentColor.rgb *
            rainbow_2(i/SAMPLES) *
            w
        );
        w *= decay;
    }

    // Smoothstepping the final color, just to bring it out a bit
    outputColor = sqrt(smoothstep(0., 1., col));
}
