#version 150

uniform sampler2D tex;
uniform vec2 dimensions;
uniform float time;
uniform float amount; // New uniform
uniform bool direction; // Vertical or horizontal
in vec2 coord;
out vec4 outputColor;

float hash21(vec2 n) {
  return fract(sin(dot(n, vec2(12.9898, 4.1414))) * 43758.5453);
}

void main()
{
    vec2 uv = coord/dimensions.xy;
    float noise;  
    if (direction) {
      noise = hash21(vec2(uv.y, time)) - .5;
    } else {
      noise = hash21(vec2(uv.x, time)) - .5;
    }
    noise *= amount; // Use the amount uniform

    if (direction) {
      uv = vec2(uv.x, (uv.y + noise));
    } else {
      uv = vec2((uv.x + noise), uv.y);
    }
    
    vec4 col = texture(tex, uv);
    
    outputColor = col;
}

