#version 150

uniform sampler2D tex;
uniform vec2 dimensions;
uniform float time;
uniform float speed;
in vec2 coord;
out vec4 outputColor;


void main()
{
  float time = time * 0.20;
  vec2 uv = coord.xy / dimensions.xy;
  vec2 p = uv;
  float d, a = atan(p.y, p.x) + time * speed;
  d = sqrt(dot(p,p));
  p = vec2(cos(a), sin(a)) * d;
  vec4 col = texture(tex, p + vec2(time * 0.01, 0.0));
  col.r = sin(col.r * 25.0 + time * 15.0);
  col.g = sin(col.g * 30.0 + time * 30.0);
  col.b = sin(col.b * 30.0 + time * 10.0);
  
  outputColor = col;
}

