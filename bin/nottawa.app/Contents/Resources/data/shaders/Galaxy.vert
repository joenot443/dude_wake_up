#version 150

uniform mat4 modelViewProjectionMatrix;
uniform vec2 dimensions;

in vec4 position;
in vec2 texcoord;

out vec2 coord;

void main(void)
{
  coord = vec2(texcoord.x, 1.0 - texcoord.y);
  gl_Position = modelViewProjectionMatrix * position;
}



