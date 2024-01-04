#version 150

uniform sampler2D tex;
uniform sampler2D tex2;
uniform vec2 dimensions;
uniform float time;
in vec2 coord;
out vec4 outputColor;
uniform float lineWidth;
uniform float dividerPosition;
uniform float angle;

void main(  )
{
    vec2 uv = coord.xy / dimensions.xy;
    
    // Calculate the line position based on dividerPosition and screen dimensions
    vec2 linePivot = vec2(dimensions.x * dividerPosition, dimensions.y * 0.5);
    
    // Translate coordinates to the pivot point
    vec2 translatedCoord = coord.xy - linePivot;

    // Rotate coordinates
    vec2 rotatedCoord;
    rotatedCoord.x = cos(angle) * translatedCoord.x - sin(angle) * translatedCoord.y;
    rotatedCoord.y = sin(angle) * translatedCoord.x + cos(angle) * translatedCoord.y;
    
    // Translate coordinates back
    rotatedCoord += linePivot;
    
    // Define line boundaries
    float lineStart = linePivot.x - lineWidth * 0.5;
    float lineEnd = linePivot.x + lineWidth * 0.5;

    // Draw the black line
    if(rotatedCoord.x >= lineStart && rotatedCoord.x <= lineEnd)
    {
        outputColor = vec4(0.0, 0.0, 0.0, 1.0);
    }
    // Sample from tex when the line is to the right
    else if(rotatedCoord.x < linePivot.x)
    {
        outputColor = vec4(texture(tex, uv));
    }
    // Sample from iChannel1 when the line is to the left
    else if(rotatedCoord.x > linePivot.x)
    {
        outputColor = vec4(texture(tex2, uv));
    }
}
