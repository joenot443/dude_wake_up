#version 150

uniform sampler2D tex;
uniform vec2 dimensions;
uniform float time;
uniform float alpha;  // Range: 0.0 to 1.0, affects color separation
uniform float beta;   // Range: 0.0 to 10.0, affects pattern frequency
uniform float gamma;  // Range: 0.0 to 2*PI, affects animation speed
in vec2 coord;
out vec4 outputColor;

#define N normalize(uv)  // Normalized uv coordinates
#define T time           // Time
#define ST sin(T)        // Sine of time

void main()
{
    // Convert fragment coordinates to normalized device coordinates
    vec2 uv = (coord * 2. - dimensions.xy) / dimensions.y;
    vec3 col;
    float l = length(uv);
    
    // Create two different UV coordinate sets for different effects
    vec2 uv1 = N * sqrt(l * l * l), uv2 = N * sin(l * beta);
    
    // Add time-based vertical displacement to uv1
    uv1.y += sin(uv1.x * cos(uv1.x + T * gamma * .7) + T);
    
    // Calculate a distance field based on trigonometric functions of uv1 and time
    float d = length(sin(uv1 + T * gamma * .8) + 2. * cos(uv1 + T * gamma * .43));
    
    // Create a smoothed, animated pattern using the distance field
    d = smoothstep(.33, .36, abs(fract(d * (1.4 + .3 * ST * beta) - T * gamma * 1.5) - (.7 + .2 * ST)));
    
    // Modify the pattern based on distance from center and sinusoidal functions
    d *= sin(d) * (.35 + .8 * sin(1.9 - length(uv)));
    
    // Create the main color using the pattern and time-based color cycling
    vec3 baseColor = vec3(d + .02 * sin(T * gamma * .7), d + .02 * sin(T * gamma * .9), d + .02 * sin(T * gamma * 1.2));
    
    // Apply color separation based on alpha
    vec2 colorShift = vec2(cos(T * gamma), sin(T * gamma)) * alpha * 0.02;
    col.r = texture(tex, coord + colorShift).r;
    col.g = baseColor.g;
    col.b = texture(tex, coord - colorShift).b;
    col = mix(baseColor, col, alpha);
    
    // If the main pattern is black, create a secondary pattern
    if (d == 0.)
    {
        // Add some high-frequency sinusoidal displacement to uv2
        uv2.x += 0.09 * sin(150. * uv.x * beta);
        uv2.y += 0.09 * sin(150. * uv.y * beta);
        
        // Add more complex, time-based vertical displacement to uv2
        uv2.y += cos(uv2.x * cos(uv2.x + T * gamma * .5) - T);
        
        // Calculate a new distance field based on uv2 and time
        d = length(cos(uv2 - T * gamma * 0.2) + 2. * sin(uv2 + T * gamma * .4));
        
        // Create a new animated pattern
        d = fract(1. * d * (1.5 + .2 * ST * beta) - T * gamma * 1.2);
        d = smoothstep(.35, .36, d - .2) + .05 * sin(T * gamma * 5.);
        
        // Modify the pattern based on distance from center
        d *= .15 + .8 * sin(1.9 - length(uv));
        d *= d;
        
        // Create a secondary color using the new pattern and time-based color cycling
        vec3 secondaryColor = vec3(d + .1 * ST, d + .1 * sin(T * gamma * 1.25), d + .1 * sin(T * gamma * 1.5));
        
        // Apply color separation to secondary pattern
        vec3 shiftedSecondary;
        shiftedSecondary.r = texture(tex, coord + colorShift * 2.0).r;
        shiftedSecondary.g = secondaryColor.g;
        shiftedSecondary.b = texture(tex, coord - colorShift * 2.0).b;
        col = mix(secondaryColor, shiftedSecondary, alpha);
    }
    
    // Output the final color
    outputColor = vec4(col, 1.);
}