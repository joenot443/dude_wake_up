#version 150

uniform sampler2D tex;
uniform sampler2D tex2;
uniform vec2 dimensions;
uniform float time;
in vec2 coord;
out vec4 outputColor;


void main()
{
  vec3 col = min(texture(tex, coord), texture(tex2, coord)).rgb;
  
  outputColor = vec4(col, 1.0);
}




