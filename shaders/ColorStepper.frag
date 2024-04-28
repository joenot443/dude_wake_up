#version 150

uniform sampler2D texx;
uniform vec2 dimensions;
uniform float time;
uniform float speed;
uniform float smoothness;
in vec2 coord;
out vec4 outputColor;


// MODIFY THIS: color stepping size
vec3 stepSize = vec3(smoothness * 1000.0);

void main(  )
{
    // Normalized pixel coordinates (from 0 to 1)
    vec2 uv = coord/dimensions.xy;

    // Time varying pixel color
    vec3 col = 0.5 + 0.5*cos(time / speed+uv.xyx+vec3(0,2,4));

    // MODIFY THIS: toggle between webcam and video textures
    vec3 tex = texture(texx, uv).rgb;
    
    col *= tex;
    
    // step the color range
    col *= stepSize;
    
    col = round(col);
    
    col /= stepSize;
    
    // Output to screen
    outputColor = vec4(col,1.0);
}
