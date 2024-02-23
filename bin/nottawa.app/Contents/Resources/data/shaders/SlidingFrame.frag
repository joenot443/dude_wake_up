#version 150

uniform sampler2D tex;
uniform sampler2D tex2;
uniform vec2 dimensions;
uniform float time;
in vec2 coord;
out vec4 outputColor;

uniform float lineWidth;
uniform float speed;

void main(  )
{
    vec2 uv = coord.xy / dimensions.xy;
    
    // Make the line move over time, looping back to the start
    float linePosition = mod(time * speed, dimensions.x * 2.0);
    
    // Check if the animation is in the first or second half of its cycle
    bool isSecondHalf = linePosition >= dimensions.x;

    // Bring the line back into screen space if it's in the second half
    if(isSecondHalf)
    {
        linePosition -= dimensions.x;
    }

    float lineStart = linePosition - lineWidth * 0.5;
    float lineEnd = linePosition + lineWidth * 0.5;

    // Draw the black line
    if(coord.x >= lineStart && coord.x <= lineEnd)
    {
        outputColor = vec4(0.0, 0.0, 0.0, 1.0);
    }
    else
    {
        if(isSecondHalf)
        {
            // Sample from iChannel1 when the line is to the right
            if(coord.x < linePosition)
            {
                outputColor = vec4(texture(tex2, uv));
            }
            // Sample from tex when the line is to the left
            else if(coord.x > linePosition)
            {
                outputColor = vec4(texture(tex, uv));
            }
        }
        else
        {
            // Sample from tex when the line is to the right
            if(coord.x < linePosition)
            {
                outputColor = vec4(texture(tex, uv));
            }
            // Sample from iChannel1 when the line is to the left
            else if(coord.x > linePosition)
            {
                outputColor = vec4(texture(tex2, uv));
            }
        }
    }
}

