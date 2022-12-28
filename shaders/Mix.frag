#version 150

uniform sampler2D tex;
uniform sampler2D tex2;

uniform float tex2_mix;
in vec2 coord;
out vec4 outputColor;

void main()
{
  vec4 tex_color = texture(tex, coord);
  vec4 tex2_color = texture(tex2, coord);
  outputColor = mix(tex_color, tex2_color, tex2_mix);
}



