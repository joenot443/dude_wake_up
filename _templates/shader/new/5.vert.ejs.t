---
to: shaders/<%= name %>.vert
---

#version 150

uniform mat4 modelViewProjectionMatrix;

// Standard vertex coordinates
in vec4 position;
// Uv coordinates
in vec2 texcoord;

out vec2 coord;

void main(void)
{
  coord = vec2(position.x, position.y);
  gl_Position = modelViewProjectionMatrix * position;
}



