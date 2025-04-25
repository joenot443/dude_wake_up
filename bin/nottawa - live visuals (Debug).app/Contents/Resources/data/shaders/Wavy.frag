#version 150

uniform sampler2D tex;
uniform vec2 dimensions;
uniform float time;
in vec2 coord;
out vec4 outputColor;

// this wave shader is based on a diagram i saw somewhere (i forget where)
// this is the closest thing i could find to it:
// https://confluence.ecmwf.int/download/attachments/340775481/Fig2.2.2A%20Decomposition%20of%20ocean%20waves%202D.png


int waveCount = 50;
float scale = 20.0;

const float PI = 3.14159265359;


// code from https://stackoverflow.com/a/4275343
float rand(vec2 co) {
    return fract(sin(dot(co + 53.2352, vec2(12.9898, 78.233))) * 43758.5453);
}

// one wave cycle is t=0 --> t=1
// returns float 0-1
float waveShape(float t) {
    //return t; // sawtooth wave, function input
    return sin(t * 2.0 * PI) * 0.5 + 0.5; // sine wave
    //return 1.0 - abs(sin(t * 2.0 * PI)); // sharp sine
}


float getHeight(vec2 uv) {
    float height = 0.0;
    for (int i = 0; i < waveCount; i++) {
        // wave attributes
        float dir = rand(vec2(i, i)) * 2.0 * PI;
        float offset = rand(vec2(i, -i));
        float width = rand(vec2(-i, i));
        
        // apply wave attributes
        float propagation = (cos(dir) * uv.x + sin(dir) * uv.y) * scale;
        propagation *= width;
        propagation += offset;
    
        // get the wave
        height += waveShape(mod(propagation + time, 1.0));
    }
    
    return height / float(waveCount);
}


// code from https://www.shadertoy.com/view/MsScRt
vec4 bumpFromDepth(vec2 uv, vec2 resolution, float scale) {
  vec2 step = 1. / resolution;
    
  float height = getHeight(uv);
    
  vec2 dxy = height - vec2(
      getHeight(uv + vec2(step.x, 0.)), 
      getHeight(uv + vec2(0., step.y))
  );
    
  return vec4(normalize(vec3(dxy * scale / step, 1.)), height);
}




// le main
void main()
{
    // normalized pixel coordinates (from 0 to 1)
    vec2 uv = coord / dimensions.xy;
    // square resolution divider
    float res = max(dimensions.x, dimensions.y);
    
    // calculated wave normal
    vec4 normal = bumpFromDepth(coord.xy/res, vec2(res), .1);
    
    outputColor = vec4(normal.rgb * 0.5 + 0.5, 1.0);
//    // Image split (mouse controlled)
//    if (coord.x > iMouse.x) {
//        uv += normal.rg * 0.05;
//        if (coord.y > iMouse.y) {
//            // final image
//            outputColor = texture(tex, uv);
//        } else {
//            // distorted image coordinates
//            outputColor = vec4(uv, 1.0, 1.0);
//        }
//    } else {
//        if (coord.y > iMouse.y) {
//            // height map
//            outputColor = vec4(vec3(getHeight(coord.xy/res)), 1.0);
//        } else {
//            // normal map
//        }
//    }
}
