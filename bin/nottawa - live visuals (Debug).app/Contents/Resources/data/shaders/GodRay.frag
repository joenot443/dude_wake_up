#version 150

in vec2 coord;
out vec4 outputColor;

void main() {
  
}
//
//uniform sampler2D tex;
//uniform vec2 dimensions;
//uniform float time;
//in vec2 coord;
//out vec4 outputColor;
//
//uniform float angle = -0.3;
//uniform float position = -0.2;
//uniform float spread = 0.5;
//uniform float cutoff = 0.1;
//uniform float falloff = 0.2;
//uniform float edge_fade = 0.15;
//
//uniform float speed = 1.0;
//uniform float ray1_density = 8.0;
//uniform float ray2_density = 30.0;
//uniform float ray2_intensity = 0.3;
//
//uniform vec4 color  = vec4(1.0, 0.9, 0.65, 0.8);
//
//uniform bool hdr = false;
//uniform float seed = 5.0;
//
//// Function to implement various blending modes
//vec4 blend(vec4 a, vec4 b, int mode) {
//  vec4 result;
//  if (mode == 0) { // Multiply
//    result = a * b;
//  } else if (mode == 1) { // Screen
//    result = 1.0 - (1.0 - a) * (1.0 - b);
//  } else if (mode == 2) { // Darken
//    result = min(a, b);
//  } else if (mode == 3) { // Lighten
//    result = max(a, b);
//  } else if (mode == 4) { // Difference
//    result = abs(a - b);
//  } else if (mode == 5) { // Exclusion
//    result = a + b - 2.0 * a * b;
//  } else if (mode == 6) { // Overlay
//    result = a.r < 0.5 ? (2.0 * a * b) : (1.0 - 2.0 * (1.0 - a) * (1.0 - b));
//  } else if (mode == 7) { // Hard light
//    result = b.r < 0.5 ? (2.0 * a * b) : (1.0 - 2.0 * (1.0 - a) * (1.0 - b));
//  } else if (mode == 8) { // Soft light
//    result = b.r < 0.5 ? (2.0 * a * b + a * a * (1.0 - 2.0 * b)) : (sqrt(a) * (2.0 * b - 1.0) + (2.0 * a) * (1.0 - b));
//  } else if (mode == 9) { // Color dodge
//    result = a / (1.0 - b);
//  } else if (mode == 10) { // Linear dodge (Add)
//    result = a + b;
//  } else if (mode == 11) { // Burn
//    result = 1.0 - (1.0 - a) / b;
//  } else if (mode == 12) { // Linear burn
//    result = a + b - 1.0;
//  } else {
//    result = a; // Default fallback
//  }
//  result.a = 1.0; // Ensure the output is fully opaque
//  return result;
//}
//
//// Random and noise functions from Book of Shader's chapter on Noise.
//float random(vec2 _uv) {
//  return fract(sin(dot(_uv.xy,
//                       vec2(12.9898, 78.233))) *
//               43758.5453123);
//}
//
//float noise (in vec2 uv) {
//  vec2 i = floor(uv);
//  vec2 f = fract(uv);
//  
//  // Four corners in 2D of a tile
//  float a = random(i);
//  float b = random(i + vec2(1.0, 0.0));
//  float c = random(i + vec2(0.0, 1.0));
//  float d = random(i + vec2(1.0, 1.0));
//  
//  
//  // Smooth Interpolation
//  
//  // Cubic Hermine Curve. Same as SmoothStep()
//  vec2 u = f * f * (3.0-2.0 * f);
//  
//  // Mix 4 coorners percentages
//  return mix(a, b, u.x) +
//  (c - a)* u.y * (1.0 - u.x) +
//  (d - b) * u.x * u.y;
//}
//
//mat2 rotate(float _angle){
//  return mat2(vec2(cos(_angle), -sin(_angle)),
//              vec2(sin(_angle), cos(_angle)));
//}
//
//vec4 screen(vec4 base, vec4 blend){
//  return 1.0 - (1.0 - base) * (1.0 - blend);
//}
//
//void main()
//{
//  
//  // Rotate, skew and move the UVs
//  vec2 transformed_uv = ( rotate(angle) * (coord.x - position) )  / ( (coord.y + spread) - (coord.y * spread) );
//  
//  // Animate the ray according the the new transformed UVs
//  vec2 ray1 = vec2(transformed_uv.x * ray1_density + sin(time * 0.1 * speed) * (ray1_density * 0.2) + seed, 1.0);
//  vec2 ray2 = vec2(transformed_uv.x * ray2_density + sin(time * 0.2 * speed) * (ray1_density * 0.2) + seed, 1.0);
//  
//  // Cut off the ray's edges
//  float cut = step(cutoff, transformed_uv.x) * step(cutoff, 1.0 - transformed_uv.x);
//  ray1 *= cut;
//  ray2 *= cut;
//  
//  // Apply the noise pattern (i.e. create the rays)
//  float rays;
//  
//  if (hdr){
//    // This is not really HDR, but check this to not clamp the two merged rays making
//    // their values go over 1.0. Can make for some nice effect
//    rays = noise(ray1) + (noise(ray2) * ray2_intensity);
//  }
//  else{
//    rays = clamp(noise(ray1) + (noise(ray2) * ray2_intensity), 0., 1.);
//  }
//  
//  // Fade out edges
//  rays *= smoothstep(0.0, falloff, (1.0 - coord.y)); // Bottom
//  rays *= smoothstep(0.0 + cutoff, edge_fade + cutoff, transformed_uv.x); // Left
//  rays *= smoothstep(0.0 + cutoff, edge_fade + cutoff, 1.0 - transformed_uv.x); // Right
//  
//  // Color to the rays
//  vec3 shine = vec3(rays) * color.rgb;
//  
//  // Try different blending modes for a nicer effect. "Screen" is included in the code,
//  // but take a look at https://godotshaders.com/snippet/blending-modes/ for more.
//  // With "Screen" blend mode:
//  shine = blend(texture(tex, coord), vec4(color), 1).rgb;
//  
//  outputColor = vec4(shine, rays * color.a);
//}
