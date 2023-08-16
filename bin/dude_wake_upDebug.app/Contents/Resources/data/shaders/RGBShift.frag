#version 150

uniform sampler2D tex;
uniform float speed;
uniform float time;
uniform float amount;

in vec2 coord;
out vec4 outputColor;

vec4 colorShift(vec4 col, vec2 uv){
  vec4 frag;
  float intensity = 0.1 * amount;
  
  frag.r = texture(tex, vec2(uv.x + sin(time * speed) * intensity,uv.y + cos(time * speed)  * intensity)).r;
  frag.g = col.g;
  frag.b = texture(tex, vec2(uv.x - sin(time * speed)  * intensity,uv.y - cos(time * speed)  * intensity)).b;
  frag.a = col.a;
  return frag;
}

void main()
{
  vec4 col = texture(tex, coord);
  
  // Output to screen
  outputColor = colorShift(col, coord);
}



