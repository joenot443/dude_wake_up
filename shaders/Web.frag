#version 150

uniform sampler2D tex;
uniform vec2 dimensions;
uniform float time;
uniform float amount;
in vec2 coord;
out vec4 outputColor;

#define pi 3.14159265359

#define ITER 40.
#define STRENGTH 0.6

bool point_is_on_line(vec2 point, vec2 line_p1, vec2 line_p2, float line_width)
{
  float a = (line_p1.y - line_p2.y) / (line_p1.x - line_p2.x);
  float b = line_p1.y - a * line_p1.x;
  
  return (
          abs(point.y - point.x * a - b) < line_width &&
          point.x >= min(line_p1.x, line_p2.x) - 0.001 &&
          point.x <= max(line_p1.x, line_p2.x) + 0.001
          );
}


bool uv_is_on_line(vec2 uv, vec2 _hold_uv, vec2 thickness, float ratio)
{
  vec2 hold_uv = _hold_uv - mod(_hold_uv, thickness);
  
  vec2 res = vec2(hold_uv.x, hold_uv.y);
  vec2 right_res = vec2(hold_uv.x + thickness.x, hold_uv.y);
  vec2 top_res = vec2(hold_uv.x, hold_uv.y + thickness.y);
  
  vec3 col = 0.9 * texture(tex, res).rgb;
  vec3 right_col = 0.9 * texture(tex, right_res).rgb;
  vec3 top_col = 0.9 * texture(tex, top_res).rgb;
  
  float norm = length(col)/sqrt(3.);
  float right_norm = length(right_col)/sqrt(3.);
  float top_norm = length(top_col)/sqrt(3.);
  
  vec2 _uv = (res - 0.5 * ratio * norm) / (1. - ratio * norm);
  vec2 right_uv = (right_res - 0.5 * ratio * right_norm) / (1. - ratio * right_norm);
  vec2 top_uv = (top_res - 0.5 * ratio * top_norm) / (1. - ratio * top_norm);
  
  return (
          point_is_on_line(uv, _uv, right_uv, 0.001)
          ||
          point_is_on_line(
                           vec2(uv.y, uv.x),
                           vec2(_uv.y, _uv.x), 
                           vec2(top_uv.y, top_uv.x), 0.0007
                           )
          );
}


void main(  )
{
  vec2 thickness = vec2(0.01, 0.01);
  
  float ratio = STRENGTH * amount;
  
  vec2 uv = coord/dimensions.xy;
  uv -= 0.5;
  uv *= (1. - 0.25 * ratio);
  uv += 0.5;
  
  vec4 text = texture(tex, uv);
  if (text.a < 0.01) {
    outputColor = vec4(0.0);
    return;
  }
  
  vec3 col = (1.0 - ratio /STRENGTH) * text.rgb;
  
  vec2 direction = 0.5 - uv;
  direction *= 1. - length(direction) * 0.1;
  
  
  for (float i=ITER; i >= 0.; --i)
  {   
    vec2 hold_uv = uv + direction * ratio * i / ITER;
    if (uv_is_on_line(uv, hold_uv, thickness, ratio)) {
      col = (1.0 + ratio * 0.5) * texture(tex, hold_uv).rgb;
      break;
    }
  }
  
  outputColor = vec4(col, 1.0);
}
