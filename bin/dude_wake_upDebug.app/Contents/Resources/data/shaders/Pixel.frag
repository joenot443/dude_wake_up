#version 150

uniform sampler2D tex;
in vec2 coord;
out vec4 outputColor;
uniform vec2 dimensions;
uniform float size;

vec2 getPixelatedSampleCoord(vec2 fragCoord)
{
  return vec2(fragCoord.x - mod(fragCoord.x, size), fragCoord.y - mod(fragCoord.y, size));
}

void main()
{
  vec2 sampleCoord = getPixelatedSampleCoord(coord*dimensions);
  outputColor = texture(tex, sampleCoord/dimensions);
}
