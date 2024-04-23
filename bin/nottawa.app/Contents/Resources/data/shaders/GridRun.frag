#version 150

uniform sampler2D tex;
uniform vec2 dimensions;
uniform float time;

uniform vec3 color;
uniform float zoom;
uniform float perspective;

in vec2 coord;
out vec4 outputColor;


float grid(vec2 uv, float battery)
{
  vec2 size = vec2(uv.y, uv.y * uv.y * 0.2) * 0.01;
  uv += vec2(0.0, time * 4.0 * (battery + 0.05));
  uv = abs(fract(uv) - 0.5);
  vec2 lines = smoothstep(size, vec2(0.0), uv);
  lines += smoothstep(size * 5.0, vec2(0.0), uv) * 0.4 * battery;
  return clamp(lines.x + lines.y, 0.0, 3.0);
}


void main(  )
{   vec2 f = coord;
  f.y = perspective * f.y; // perspective
  vec2 uv = (2.0 * f.xy - dimensions.xy)/dimensions.y;
  float battery = 1.0;
  // Grid
  float fog = smoothstep(0.1, -0.02, abs(uv.y + 0.2));
  vec3 col = vec3(0.0, 0.1, 0.2);
  
  uv.y = 5.0 / (abs(uv.y + 0.2) + zoom); // zoom
  uv.x *= uv.y * 1.0;
  float gridVal = grid(uv, battery);
  col = mix(col, color.xyz, gridVal); // color
  
  col += fog * fog * fog;
  col = mix(vec3(col.r, col.r, col.r) * 0.5, col, battery * 0.7);
  
  outputColor = vec4(col,1.0);
}

