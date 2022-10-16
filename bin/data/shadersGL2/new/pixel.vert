#version 120

varying vec2 coord;

void main(void)
{
  coord = gl_MultiTexCoord0.xy;
  gl_Position = ftransform();
}

