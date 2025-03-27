#version 150

uniform sampler2D tex;
uniform vec2 dimensions;
uniform vec3 color;
uniform float colorMix;
uniform float time;
in vec2 coord;
out vec4 outputColor;


void main()
{
  vec2 uv = coord.xy / dimensions.xy;
  vec2 pos = (uv.xy - 0.5);
  vec2 cir = ((pos.xy * pos.xy + sin(uv.x * 18.0 + time) / 25.0 * sin(uv.y * 7.0 + time * 1.5) / 1.0) + uv.x * sin(time) / 16.0 + uv.y * sin(time * 1.2) / 16.0);
  float circles = (sqrt(abs(cir.x + cir.y * 0.5) * 25.0) * 5.0);
  
  vec3 o = mix(color, vec3(sin(circles * 1.25 + 2.0), abs(sin(circles * 1.0 - 1.0) - sin(circles)), abs(sin(circles) * 1.0)), 1.0 - colorMix);
  
  outputColor = vec4(o, 1.0);
}
