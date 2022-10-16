#version 120

uniform sampler2DRect tex;
varying vec2 coord;
uniform vec2 dimensions;
uniform float size;

vec2 getPixelatedSampleCoord(vec2 fragCoord)
{
  return vec2(fragCoord.x - mod(fragCoord.x, size), fragCoord.y - mod(fragCoord.y, size));
}

void main()
{
    vec2 sampleCoord = getPixelatedSampleCoord(coord);
    gl_FragColor = texture2DRect(tex, sampleCoord);
}
