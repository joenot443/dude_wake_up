#version 150

uniform sampler2D tex;
uniform vec2 dimensions;
uniform float time;
in vec2 coord;
out vec4 outputColor;


void main()
{
  vec4 tex_color=texture(tex, coord);
  float a = 0.0;
  if (tex_color.a > 0.1) {
    a = tex_color.a;
  }
  outputColor = vec4(tex_color.x, tex_color.y, tex_color.z, a);
}



