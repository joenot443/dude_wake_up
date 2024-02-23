#version 150

uniform sampler2D tex;
uniform vec2 dimensions;
uniform float time;
in vec2 coord;
out vec4 outputColor;

const float PIXEL_FACTOR = 320.;
const float COLOR_FACTOR = 3.;

//wiki Ordered dithering table
const mat4 ditherTable = mat4(
    0.0, 8.0, 2.0, 10.0,
    12.0, 4.0, 14.0, 6.0,
    3.0, 11.0, 1.0, 9.0,
    15.0, 7.0, 13.0, 5.0
);


void main(  )
{
    // Reduce pixels
    vec2 size = PIXEL_FACTOR * dimensions.xy/dimensions.x;
    vec2 coor = floor( coord/dimensions.xy * size) ;
    vec2 uv = coor / size;
                
     // Get source color
    vec3 col = texture(tex, uv).xyz;
    
    // Dither
    col += ditherTable[int( coor.x ) % 4][int( coor.y ) % 4] * 0.015; // last number is dithering strength

    // Reduce colors
    col = floor(col * COLOR_FACTOR) / COLOR_FACTOR ;
    
    // Output to screen
    outputColor = vec4(col,1.);
}
