#version 150

uniform sampler2D tex;
uniform vec2 dimensions;
uniform float time;
uniform float amount;

in vec2 coord;
out vec4 outputColor;

void main() {
  // Normalize to [0,1]
  vec2 p = coord.xy / dimensions.xy;
  vec2 center = vec2(0.5, 0.5);
  float aspectRatio = dimensions.x / dimensions.y;

  // Vector from center, corrected for aspect ratio
  vec2 d = (p - center) * vec2(aspectRatio, 1.0);
  float r = length(d);

  if (r == 0.0 || amount < 0.001) {
    outputColor = vec4(texture(tex, p).rgb, 1.0);
    return;
  }

  // Fisheye distortion using power curve.
  // amount=0: no effect (exponent=1), amount=1: strong barrel/pop-out (exponent=3)
  float maxR = length(vec2(0.5 * aspectRatio, 0.5));
  float rNorm = r / maxR;
  float exponent = 1.0 + amount * 2.0;
  float newR = pow(rNorm, exponent) * maxR;

  // Map back to texture space
  vec2 distorted = normalize(d) * newR / vec2(aspectRatio, 1.0);
  vec2 uv = center + distorted;

  outputColor = vec4(texture(tex, uv).rgb, 1.0);
}
