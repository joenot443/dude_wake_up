#version 150

uniform sampler2D newTex;
uniform sampler2D baseTex;
uniform vec2 dimensions;
uniform float time;
in vec2 coord;
out vec4 outputColor;


void main()
{
  vec4 newTexColor = texture(newTex, coord);
  vec4 baseTexColor = texture(baseTex, coord);
  bool drawNewTex = false;
  
  if (baseTexColor.a < 0.1) {
    outputColor = newTexColor;
  } else {
    outputColor = baseTexColor;
  }
}
