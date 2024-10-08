#version 150

uniform sampler2D tex;
uniform vec2 dimensions;
uniform float time;
uniform float colorMix;
uniform int colorTransparentPixels;
uniform vec4 color;
in vec2 coord;
out vec4 outputColor;


void main()
{
  vec4 outp = color;
  vec4 t = texture(tex, coord);
  if (colorTransparentPixels != 1) {
    outp.a = texture(tex, coord).r;
  }
  outputColor = mix(t, outp, colorMix);
}



