#version 150

uniform sampler2D tex;
uniform sampler2D tex2;
uniform sampler2D tex3;
uniform vec2 dimensions;
uniform float time;
in vec2 coord;
out vec4 outputColor;

#define CHAOS_FILL 0 // set to 1 for more interesting fill patterns.  Looks very nice with color_to_val_3

const float diffuse_weight = 0.35; // higher means less diffusion.  set greater than 0 and less than 1
const float react_weight = 0.2; // reaction speed : don't exceed 0.25.  higher is faster reaction

// in the function "greatest" there are two calls to "color_to_val_4"
// change those to "color_to_val_1", "color_to_val_2" etc to get different effects

vec2 to_uv(in vec2 in_pixels, in vec2 offset) {
  return mod(in_pixels + offset, iResolution.xy) / iResolution.xy;
  // return 0.1 + mod(vec2(0.8) + in_pixels / iResolution.xy, vec2(0.9));
}




void mainImage()
{

  vec4 color = texture(tex2, to_uv(coord, vec2(0.0)));

  // color.rg = mod(10.0 * to_uv(p), vec2(1.0));
  color.b = 1.0 - 0.5 * color.r - 0.5 * color.g;




  color = diffuse_weight * texture(tex3, to_uv(p, vec2(0.0)));

  color.rgb = mix(smoothstep(vec3(0.0), vec3(1.0), texture(tex3, to_uv(p, vec2(0.0))).rgb), color.rgb, 0.95);


  float w = 0.25 * (1.0 - diffuse_weight);
  color.rgb = color.rgb +  w * texture(tex, to_uv(p, vec2(1.1, 0.0))).rgb;
  color.rgb = color.rgb +  w * texture(tex, to_uv(p, vec2(0.0, 1.1))).rgb;
  color.rgb = color.rgb +  w * texture(tex, to_uv(p, vec2(-1.1, 0.0))).rgb;
  color.rgb = color.rgb +  w * texture(tex, to_uv(p, vec2(0.0, -1.1))).rgb;


  color.rgb = color.rgb + react_weight * (1.0 * color.rgb * color.gbr - color.rgb * color.brg);//  + 0.005;

  color.a = 1.0;

  color.rgb = max(vec3(0.0), min(vec3(1.0), color.rgb));

  float delta = 1.5 - dot(color.rgb, vec3(1.0));

  color += 0.005 * delta;

  outputColor = color;
}
