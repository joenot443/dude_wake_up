#version 150

uniform sampler2D tex;
uniform vec2 dimensions;
uniform float time;
uniform float blurRadius;
uniform float channelSeparation;
uniform float threshold;
uniform float rotation;
uniform float spiral;
uniform float hueShift;
uniform float chromaticStrength;
in vec2 coord;
out vec4 outputColor;

// RGB to HSV conversion
vec3 rgb2hsv(vec3 c) {
    vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
    vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
    vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));
    float d = q.x - min(q.w, q.y);
    float e = 1.0e-10;
    return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
}

// HSV to RGB conversion
vec3 hsv2rgb(vec3 c) {
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}


void main() {
  float t = time;
  vec2 p = coord / dimensions.xy;

  // Calculate blur offsets for each channel based on separation and chromatic strength
  float separationAmount = channelSeparation * chromaticStrength;

  float rBlurX = blurRadius * (1.0 - 0.8 * separationAmount);
  float rBlurY = blurRadius;
  float gBlurX = blurRadius;
  float gBlurY = blurRadius;
  float bBlurX = blurRadius;
  float bBlurY = blurRadius * (1.0 - 0.8 * separationAmount);

  // Threshold range for smoothstep
  float thresholdLow = threshold - 0.05;
  float thresholdHigh = threshold + 0.05;

  // Red channel
  float r = 0.0;
  float pass = 0.0;

  for (int i = 0; i < 50; i++) {
    float angle = pass + rotation + (pass * spiral * 0.1);

    vec2 sampleCoord = vec2(p.x + cos(angle) * rBlurX, p.y + sin(angle) * rBlurY);
    vec3 sampledColor = texture(tex, sampleCoord).rgb;

    // Apply hue shift
    if (hueShift > 0.0) {
      vec3 hsv = rgb2hsv(sampledColor);
      hsv.x = fract(hsv.x + (pass / 50.0) * hueShift);
      sampledColor = hsv2rgb(hsv);
    }

    r += smoothstep(thresholdLow, thresholdHigh, sampledColor.r);
    pass += 1.0;
  }
  r /= pass;

  // Green channel
  float g = 0.0;
  pass = 0.0;

  for (int i = 0; i < 50; i++) {
    float angle = pass + rotation + (pass * spiral * 0.1);

    vec2 sampleCoord = vec2(p.x + cos(angle) * gBlurX, p.y + sin(angle) * gBlurY);
    vec3 sampledColor = texture(tex, sampleCoord).rgb;

    if (hueShift > 0.0) {
      vec3 hsv = rgb2hsv(sampledColor);
      hsv.x = fract(hsv.x + (pass / 50.0) * hueShift);
      sampledColor = hsv2rgb(hsv);
    }

    g += smoothstep(thresholdLow, thresholdHigh, sampledColor.g);
    pass += 1.0;
  }
  g /= pass;

  // Blue channel
  float b = 0.0;
  pass = 0.0;

  for (int i = 0; i < 50; i++) {
    float angle = pass + rotation + (pass * spiral * 0.1);

    vec2 sampleCoord = vec2(p.x + cos(angle) * bBlurX, p.y + sin(angle) * bBlurY);
    vec3 sampledColor = texture(tex, sampleCoord).rgb;

    if (hueShift > 0.0) {
      vec3 hsv = rgb2hsv(sampledColor);
      hsv.x = fract(hsv.x + (pass / 50.0) * hueShift);
      sampledColor = hsv2rgb(hsv);
    }

    b += smoothstep(thresholdLow, thresholdHigh, sampledColor.b);
    pass += 1.0;
  }
  b /= pass;

  // Calculate alpha based on the blur effect intensity
  // Use the maximum of RGB channels to represent the overall blur effect
  // This ensures the blur area (including the spread) is captured in alpha
  float alpha = max(max(r, g), b);
  
  outputColor = vec4(r, g, b, alpha);
}
