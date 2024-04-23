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
  
  if (tex_color.a < 0.1 && tex2_color.a < 0.1) {
    outputColor = vec4(0.0);
    return;
  }
  
  if (tex2_color.a < 0.1) {
    outputColor = vec4(tex_color.rgb, tex_color.a);
    return;
  } else if (tex_color.a < 0.1) {
    outputColor = vec4(tex2_color.rgb, tex2_color.a);
    return;
  }
  
  outputColor = mix(tex2_color, tex_color, tex2_mix);
}
