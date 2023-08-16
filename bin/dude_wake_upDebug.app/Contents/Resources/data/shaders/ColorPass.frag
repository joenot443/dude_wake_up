#version 150

uniform sampler2D tex;
uniform vec2 dimensions;
uniform float time;
uniform bool invert;
uniform float hue1;
uniform float hue2;

in vec2 coord;
out vec4 outputColor;

// Function definition
vec4 rgb_to_hsv(vec4);

void main() {
  // Retrieve color texture
  vec4 texColor = texture(tex, coord);

  // Determine HSV and gray color
  vec4 hsv = rgb_to_hsv(texColor);
  vec4 grayMask =
      vec4(vec3((texColor.r + texColor.g + texColor.b) / 3.0f), texColor.a);

  // Determine final color
  if (!invert && (hsv.r > hue1 && hsv.r < hue2))
    outputColor = texColor;
  else if (invert && (hsv.r < hue1 || hsv.r > hue2))
    outputColor = texColor;
  else
    outputColor = grayMask;
}

vec4 rgb_to_hsv(vec4 color) {
  vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
  vec4 p =
      mix(vec4(color.bg, K.wz), vec4(color.gb, K.xy), step(color.b, color.g));
  vec4 q = mix(vec4(p.xyw, color.r), vec4(color.r, p.yzx), step(p.x, color.r));

  float d = q.x - min(q.w, q.y);
  float e = 1.0e-10;
  return vec4(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x,
              color[3]);
}
