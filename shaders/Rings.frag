#version 150

uniform sampler2D tex;
uniform vec2 dimensions;
uniform vec3 color;
uniform float time;
uniform int ringCount;
uniform float ringSize;

#define M_PI 3.1415926535897932384626433832795

in vec2 coord;
out vec4 outputColor;

void main() {
  vec2 vTextureCoord = coord;
  float width = dimensions.x;
  float height = dimensions.y;
  float t = time * 1000.0;

  vec2 ratio = vec2(width, height) / width;
  vec2 pos = vTextureCoord * ratio;
  float scale = (ringSize + sin(t / 25000.0)) / float(ringCount);
  pos += vec2(sin(t / 3500.0), sin(t / 3650.0)) * 0.1;
  vec2 centre = vec2(0.1 * scale, 0.1 * scale);

  float radius1 = distance(
      vec2(mod(sin(sin((0.5 * M_PI * pos.y) + t / 3800.0) + pos.x * M_PI),
               0.2 * scale),
           mod(sin(cos((0.5 * M_PI * pos.x) + t / 4100.0) + pos.y * M_PI),
               0.2 * scale)),
      centre);

  float angle1 = atan(pos.x - 0.5, pos.y - 0.5);

  float angle2 =
      atan(mod(sin(sin((0.5 * M_PI * pos.y) + t / 3800.0) + pos.x * M_PI),
               0.2 * scale) -
               centre.x,
           mod(sin(cos((0.5 * M_PI * pos.x) + t / 4100.0) + pos.y * M_PI),
               0.2 * scale) -
               centre.y);

  float red = 0.0;
  float green = 0.0;
  float blue = 0.0;
  if ((radius1 < 0.09 * scale) && (radius1 > 0.06 * scale)) {
    red = 1.0 - (sin((t / 3300.0) + angle2 * 3.0) + 1.0) / 2.0 * color.r;
    green = (sin((t / 3200.0) + angle2 * 3.0) + 1.0) / 2.0 * color.g;
    blue = (sin((t / 3200.0) + angle1 * 3.0) + 1.0) / 2.0 * color.b;
  } else {
    red = color.r;
    green = color.g;
    blue = color.b;

    vec2 shadow = vec2(+0.005, -0.005);
    float radius1 = distance(
        vec2(mod(sin(sin((0.5 * M_PI * (pos.y - shadow.y)) + t / 3800.0) +
                     (pos.x - shadow.x) * M_PI),
                 0.2 * scale),
             mod(sin(cos((0.5 * M_PI * (pos.x - shadow.x)) + t / 4100.0) +
                     (pos.y - shadow.y) * M_PI),
                 0.2 * scale)),
        centre);

    if ((radius1 < 0.09 * scale) && (radius1 > 0.06 * scale)) {
      float darkness = cos(M_PI * (((radius1 / scale) - 0.075) / 0.03));
      red = color.r - 0.5 * darkness;
      green = color.g - 0.5 * darkness;
      blue = color.b - 0.5 * darkness;
    }
  }

  vec4 colour = vec4(red, green, blue, 1.0);

  outputColor = colour;
}
