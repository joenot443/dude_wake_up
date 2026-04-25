#version 150

uniform sampler2D tex;
uniform vec2 dimensions;
uniform float time;
uniform float frequency;
uniform float phase;
uniform float phaseSpread;
uniform float drive;
uniform float bias;
uniform float mixAmount;
in vec2 coord;
out vec4 outputColor;

const float TWO_PI = 6.28318530718;

// One channel of solarized sine tone-mapping.
//   shaped = pow(input, drive)        - response curve
//   wave   = sin((shaped + bias) * f * 2pi + phase)
//   out    = 0.5 + 0.5 * wave
float solarize(float v, float channelPhase) {
  float shaped = pow(clamp(v, 0.0, 1.0), drive);
  float arg = (shaped + bias) * frequency * TWO_PI + channelPhase;
  return 0.5 + 0.5 * sin(arg);
}

void main() {
  vec2 uv = coord / dimensions.xy;
  vec4 src = texture(tex, uv);

  // Per-channel phase: green leads by spread, blue by 2*spread.
  // At spread = 0 the channels solarize identically -> pure tone wraps.
  // At spread != 0 the bands shift hue between channels.
  float pR = phase;
  float pG = phase + phaseSpread;
  float pB = phase + 2.0 * phaseSpread;

  vec3 graded = vec3(
    solarize(src.r, pR),
    solarize(src.g, pG),
    solarize(src.b, pB)
  );

  outputColor = vec4(mix(src.rgb, graded, mixAmount), src.a);
}
