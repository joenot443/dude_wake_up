#version 150

uniform mat4 modelViewProjectionMatrix;

// Standard vertex coordinates
in vec4 position;
// Uv coordinates
in vec2 texcoord;

out vec2 coord;

void main(void)
{
  coord = vec2(texcoord.x, texcoord.y);
  gl_Position = modelViewProjectionMatrix * texcoord;
}
