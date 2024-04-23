#version 150

uniform sampler2D tex;
uniform sampler2D tex2;
uniform vec2 dimensions;
uniform float time;
in vec2 coord;
out vec4 outputColor;


void main()
{
  vec4 tex_color = texture(tex, coord);
  outputColor = vec4(tex_color.x, tex_color.y, tex_color.z, 1.0);
}



