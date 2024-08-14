#version 150

uniform sampler2D tex;
uniform vec2 dimensions;
uniform float time;
uniform float cycles;
uniform int palette;
in vec2 coord;
out vec4 outputColor;

vec3 palette_vec(float t) {
    vec3 a, b, c, d;

    if (palette == 0) {
      a = vec3(0.5, 0.5, 0.5);
      b = vec3(0.9, 0.5, 0.5);
      c = vec3(1.0, 1.0, 1.0);
      d = vec3(0.263, 0.416, 0.557);
    } else if (palette == 1) {
        a = vec3(0.5, 0.5, 0.5);
        b = vec3(0.5, 0.5, 0.5);
        c = vec3(1.0, 1.0, 1.0);
        d = vec3(0.00, 0.10, 0.20);
    } else if (palette == 2) {
        a = vec3(0.5, 0.5, 0.5);
        b = vec3(0.5, 0.5, 0.5);
        c = vec3(1.0, 1.0, 1.0);
        d = vec3(0.30, 0.20, 0.20);
    } else if (palette == 3) {
        a = vec3(0.5, 0.5, 0.5);
        b = vec3(0.5, 0.5, 0.5);
        c = vec3(1.0, 1.0, 0.5);
        d = vec3(0.80, 0.90, 0.30);
    } else if (palette == 4) {
        a = vec3(0.5, 0.5, 0.5);
        b = vec3(0.5, 0.5, 0.5);
        c = vec3(1.0, 0.7, 0.4);
        d = vec3(0.00, 0.15, 0.20);
    } else if (palette == 5) {
        a = vec3(0.5, 0.5, 0.5);
        b = vec3(0.5, 0.5, 0.5);
        c = vec3(2.0, 1.0, 0.0);
        d = vec3(0.50, 0.20, 0.25);
    } else if (palette == 6) {
        a = vec3(0.8, 0.5, 0.4);
        b = vec3(0.2, 0.4, 0.2);
        c = vec3(2.0, 1.0, 1.0);
        d = vec3(0.00, 0.25, 0.25);
    } else {
      a = vec3(0.5, 0.5, 0.5);
      b = vec3(0.5, 0.5, 0.5);
      c = vec3(1.0, 1.0, 1.0);
      d = vec3(0.00, 0.33, 0.67);
    }

    return a + b * cos(6.28318 * (c * t + d));
}

//https://www.shadertoy.com/view/mtyGWy
void main(  ) {
    vec2 uv = (coord * 2.0 - dimensions.xy) / dimensions.y;
    vec2 uv0 = uv;
    vec3 finalColor = vec3(0.0);
    
    for (float i = 0.0; i < cycles; i++) {
        uv = fract(uv * 1.5) - 0.5;

        float d = length(uv) * exp(-length(uv0));

        vec3 col = palette_vec(length(uv0) + i*.4 + time*.4);

        d = sin(d*8. + time)/8.;
        d = abs(d);

        d = pow(0.01 / d, 1.2);

        finalColor += col * d;
    }
        
    outputColor = vec4(finalColor, 1.0);
}
