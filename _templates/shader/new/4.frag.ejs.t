---
to: bin/data/shaders/<%= name %>.frag
---

#version 150

uniform sampler2D tex;
in vec2 coord;
out vec4 outputColor;

void main()
{
  vec4 tex_color=texture(tex, coord);
  outputColor = vec4(tex_color.x, tex_color.y, tex_color.z, 1.0);
}


