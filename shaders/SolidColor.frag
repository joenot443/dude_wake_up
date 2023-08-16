#version 150

uniform sampler2D tex;
uniform vec2 dimensions;
uniform float time;
uniform vec4 color;
in vec2 coord;
out vec4 outputColor;


void main()
{
  outputColor = color;
}



