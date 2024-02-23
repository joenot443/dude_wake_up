#version 150

uniform sampler2D tex;
uniform vec2 dimensions;
uniform float time;
uniform float amount;

in vec2 coord;
out vec4 outputColor;

// Adapted from http://strlen.com/gfxengine/fisheyequake/
vec2 fisheye_lookup(float fov, vec2 position) {
  vec2 d = position - 0.5;

  float yaw = sqrt(d.x * d.x + d.y * d.y) * fov;

  float roll = -atan(d.y, d.x);
  float sx = sin(yaw) * cos(roll);
  float sy = sin(yaw) * sin(roll);
  float sz = cos(yaw);

  return vec2(sx, sy);
}

// lens
void main() {
  vec2 p = coord.xy / dimensions.xy;
  vec2 a = vec2(amount);
  vec2 m = a / dimensions.xy;
  float lensSize = 2.0;
  float scale = 1.0;

  vec2 d = p - m;
  float r = scale * sqrt(dot(d, d)); // distance of pixel from mouse
  float rThresh = 0.3;

  vec2 pos = d;
  float apertureHalf = 0.5 * 90.0 * (3.14159 / 180.0);
  float maxFactor = sin(apertureHalf);

  vec2 uv;

  float avgD = (dimensions.x + dimensions.y) / 2.0;
  vec2 fMinMouse = coord.xy / avgD - a / dimensions.xy;
  float r2 =
      scale * sqrt(dot(fMinMouse, fMinMouse)); // distance of pixel from mouse

  if (r2 >= lensSize) {
    outputColor = vec4(0, 0, 0, 1.0);
  } else {
    // transition from inverse to full fisheye
    vec2 p2 = coord.xy / dimensions.x; // normalized coords with some cheat
                                       //(assume 1:1 prop)
    float prop = dimensions.x / dimensions.y;
    vec2 m2 = vec2(0.5, 0.5 / prop); // center coords
    // vec2 m2 = vec2(m.x, 0.5 / prop);//center coords
    vec2 d2 = p2 - m2;            // vector from center to current fragment
    float r2 = sqrt(dot(d2, d2)); // distance of pixel from center

    float power = (2.0 * 3.141592 / (2.0 * sqrt(dot(m2, m2)))) *
                  (m.x - 0.5); // amount of effect

    float bind; // radius of 1:1 effect
    if (power > 0.0)
      bind = sqrt(dot(m2, m2)); // stick to corners
    else {
      if (prop < 1.0)
        bind = m2.x;
      else
        bind = m2.y;
    } // stick to borders

    // eqn https://www.shadertoy.com/view/4s2GRR
    if (power > 0.0) // fisheye
      uv = m2 + normalize(d2) * tan(r2 * power) * bind / tan(bind * power);
    else if (power < 0.0) // antifisheye
      uv = m2 + normalize(d2) * atan(r2 * -power * 10.0) * bind /
                    atan(-power * bind * 10.0);
    else
      uv = p; // no effect for power = 1.0

    vec3 col = texture(tex, vec2(uv.x, -uv.y * prop))
                   .xyz; // Second part of cheat
                         // for round effect, not elliptical
    outputColor = vec4(col, 1.0);
  }
}
