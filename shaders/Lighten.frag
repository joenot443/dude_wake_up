#version 150

uniform sampler2D tex;
uniform vec2 dimensions;
uniform float time;
uniform float amount;
in vec2 coord;
out vec4 outputColor;

void main()
{
  vec4 src = texture(tex, coord);
  vec3 lit = mix(src.rgb, vec3(1.0), amount);
  outputColor = vec4(lit, src.a);
}
