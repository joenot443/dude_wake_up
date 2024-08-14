#version 150

uniform sampler2D tex;
uniform sampler2D tex2;
uniform vec2 dimensions;
uniform float time;
in vec2 coord;
out vec4 outputColor;

void main(  )
{
  vec2 uv = coord.xy / dimensions.xy;
  vec2 block = floor(coord.xy / vec2(16));
  vec2 uv_noise = block / vec2(64);
  uv_noise += floor(vec2(time) * vec2(1234.0, 3543.0)) / vec2(64);
  
  float block_thresh = pow(fract(time * 1236.0453), 2.0);
  float line_thresh = pow(fract(time * 2236.0453), 3.0) * 0.7;
  
  vec2 uv_r = uv;
  vec2 uv_g = uv;
  vec2 uv_b = uv;

  // glitch some blocks and lines
  if (texture(tex2, uv_noise).r < block_thresh ||
    texture(tex2, vec2(uv_noise.y, 0.0)).g < line_thresh) {

    vec2 dist = (fract(uv_noise) - 0.5) * 0.3;
    uv_r += dist * 0.1;
    uv_g += dist * 0.2;
    uv_b += dist * 0.125;
  }

  outputColor.r = texture(tex, uv_r).r;
  outputColor.g = texture(tex, uv_g).g;
  outputColor.b = texture(tex, uv_b).b;
  outputColor.a = 1.0;

  // loose luma for some blocks
  if (texture(tex2, uv_noise).g < block_thresh)
    outputColor.rgb = outputColor.ggg;

  // discolor block lines
  if (texture(tex2, vec2(uv_noise.y, 0.0)).b * 3.5 < line_thresh)
    outputColor.rgb = vec3(0.0, dot(outputColor.rgb, vec3(1.0)), 0.0);

  // interleave lines in some blocks
  if (texture(tex2, uv_noise).g * 1.5 < block_thresh ||
    texture(tex2, vec2(uv_noise.y, 0.0)).g * 2.5 < line_thresh) {
    float line = fract(coord.y / 3.0);
    vec3 mask = vec3(3.0, 0.0, 0.0);
    if (line > 0.333)
      mask = vec3(0.0, 3.0, 0.0);
    if (line > 0.666)
      mask = vec3(0.0, 0.0, 3.0);
    
    outputColor.xyz *= mask;
  }
}
