#version 150

uniform sampler2D tex;
uniform vec2 dimensions;
uniform float time;
uniform float amount; // New uniform
in vec2 coord;
out vec4 outputColor;

float hash21(vec2 n) {
  return fract(sin(dot(n, vec2(12.9898, 4.1414))) * 43758.5453);
}

void main()
{
    vec2 uv = coord/dimensions.xy;

    float noise = hash21(vec2(uv.y, time)) - .5;
    noise *= amount; // Use the amount uniform

    uv = vec2((uv.x + noise), uv.y);
    
    vec3 col = texture(tex, uv).rgb;
    
    outputColor = vec4(col, 1.);
}

