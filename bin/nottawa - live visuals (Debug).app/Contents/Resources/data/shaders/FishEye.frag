#version 150

uniform sampler2D tex;
uniform vec2 dimensions;
uniform float time;
uniform float amount;

in vec2 coord;
out vec4 outputColor;

//Inspired by http://stackoverflow.com/questions/6030814/add-fisheye-effect-to-images-at-runtime-using-opengl-es
void main(  )//Drag mouse over rendering area
{
  vec2 p = coord.xy / dimensions.x;//normalized coords with some cheat
                                                           //(assume 1:1 prop)
  float prop = dimensions.x / dimensions.y;//screen proroption
  vec2 m = vec2(0.5, 0.5 / prop);//center coords
  vec2 d = p - m;//vector from center to current fragment
  float r = sqrt(dot(d, d)); // distance of pixel from center

  float power = ( 2.0 * 3.141592 / (2.0 * sqrt(dot(m, m))) ) *
        (amount - 0.5);//amount of effect

  float bind;//radius of 1:1 effect
  if (power > 0.0) bind = sqrt(dot(m, m));//stick to corners
  else {if (prop < 1.0) bind = m.x; else bind = m.y;}//stick to borders

  //Weird formulas
  vec2 uv;
  if (power > 0.0)//fisheye
    uv = m + normalize(d) * tan(r * power) * bind / tan( bind * power);
  else if (power < 0.0)//antifisheye
    uv = m + normalize(d) * atan(r * -power * 10.0) * bind / atan(-power * bind * 10.0);
  else uv = p;//no effect for power = 1.0

  vec3 col = texture(tex, uv).xyz;//Second part of cheat
                                                    //for round effect, not elliptical
  outputColor = vec4(col, 1.0);
}
