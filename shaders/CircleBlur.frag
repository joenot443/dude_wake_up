#version 150

uniform sampler2D tex;
uniform vec2 dimensions;
uniform float time;
uniform float blurRadius;
uniform float channelSeparation;
uniform float threshold;
in vec2 coord;
out vec4 outputColor;


void main() {
  float t = time;
  vec2 p = coord / dimensions.xy;

  // Calculate blur offsets for each channel based on separation
  // Original had: R(0.01, 0.05), G(0.05, 0.05), B(0.05, 0.01)
  float rBlurX = blurRadius * (0.2 + 0.8 * (1.0 - channelSeparation));
  float rBlurY = blurRadius;
  float gBlurX = blurRadius;
  float gBlurY = blurRadius;
  float bBlurX = blurRadius;
  float bBlurY = blurRadius * (0.2 + 0.8 * (1.0 - channelSeparation));

  // Threshold range for smoothstep
  float thresholdLow = threshold - 0.05;
  float thresholdHigh = threshold + 0.05;

  // Red channel
  float r = 0.0;
  float pass = 0.0;

  for (int i = 0; i < 50; i++) {
    r += smoothstep(thresholdLow, thresholdHigh,
                    texture(tex, vec2(p.x + cos(pass)*rBlurX, p.y + sin(pass)*rBlurY)).r);
    pass += 1.0;
  }
  r /= pass;

  // Green channel
  float g = 0.0;
  pass = 0.0;

  for (int i = 0; i < 50; i++) {
    g += smoothstep(thresholdLow, thresholdHigh,
                    texture(tex, vec2(p.x + cos(pass)*gBlurX, p.y + sin(pass)*gBlurY)).g);
    pass += 1.0;
  }
  g /= pass;

  // Blue channel
  float b = 0.0;
  pass = 0.0;

  for (int i = 0; i < 50; i++) {
    b += smoothstep(thresholdLow, thresholdHigh,
                    texture(tex, vec2(p.x + cos(pass)*bBlurX, p.y + sin(pass)*bBlurY)).b);
    pass += 1.0;
  }
  b /= pass;

  outputColor = vec4(r, g, b, 1.0);
}
