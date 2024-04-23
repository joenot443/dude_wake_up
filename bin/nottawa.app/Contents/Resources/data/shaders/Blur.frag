#version 150

// Add uniforms to select channels
uniform sampler2D tex;
uniform vec2 dimensions;
uniform float size;

in vec2 coord;
out vec4 outputColor;

void main(  )
{
    float Pi = 6.28318530718; // Pi*2
    
    // GAUSSIAN BLUR SETTINGS {{{
    float Directions = 16.0; // BLUR DIRECTIONS (Default 16.0 - More is better but slower)
    float Quality = 3.0; // BLUR QUALITY (Default 4.0 - More is better but slower)
    float Size = size; // BLUR SIZE (Radius)
    // GAUSSIAN BLUR SETTINGS }}}
   
    vec2 Radius = Size/dimensions.xy;
    
    // Normalized pixel coordinates (from 0 to 1)
    vec2 uv = coord/dimensions.xy;
    // Pixel colour
    vec4 Color = texture(tex, uv);
    
    // Blur calculations
    for( float d=0.0; d<Pi; d+=Pi/Directions)
    {
    for(float i=1.0/Quality; i<=1.0; i+=1.0/Quality)
        {
      Color += texture( tex, uv+vec2(cos(d),sin(d))*Radius*i);
        }
    }
    
    // Output to screen
    Color /= Quality * Directions - 15.0;
    outputColor =  Color;
}
