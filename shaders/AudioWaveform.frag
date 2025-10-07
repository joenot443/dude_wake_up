#version 150

uniform sampler2D tex;
uniform float audio[256];
uniform float waveform[256];
uniform float time;
uniform float thickness;
uniform int lineCount;
uniform float amplitude;
uniform float colorShift;
uniform float glowIntensity;

in vec2 coord;
out vec4 outputColor;

float squared(float value) { return value * value; }

float getAmp(float frequency) { return audio[int(frequency)]; }

float getWeight(float f) {
  return (+getAmp(f - 2.0) + getAmp(f - 1.0) + getAmp(f + 2.0) +
          getAmp(f + 1.0) + getAmp(f)) /
         5.0;
}

void main() {
  vec2 uv = -1.0 + 2.0 * coord;

  float lineIntensity;
  float glowWidth;
  vec3 color = vec3(0.0);

  for (float i = 0.0; i < float(lineCount); i++) {

    uv.y += (0.2 * sin(uv.x + i / 7.0 - time * 0.6));

    // Each layer gets its own frequency band (bass at bottom, treble at top)
    float layerBand = clamp((i + 0.5) * (256.0 / float(lineCount)), 0.0, 255.0);
    float layerFreq = getWeight(layerBand) - 0.5;

    // Use actual waveform data for horizontal variation (oscilloscope-style)
    float xWave = waveform[int(coord.x * 255.0)];

    // Blend: 75% layer-based frequency, 25% waveform oscillation
    float a = mix(xWave, layerFreq, 0.75) * amplitude;

    float Y = uv.y + a;

    lineIntensity =
        0.4 + squared(1.6 * abs(mod(coord.x + i / 1.3 + time, 2.0) - 1.0));
    glowWidth = abs(lineIntensity / ((150.0 / thickness) * Y));
    color += vec3(glowWidth * (2.0 + sin(time * 0.13 + colorShift)),
                  glowWidth * (2.0 - sin(time * 0.23 + colorShift)),
                  glowWidth * (2.0 - cos(time * 0.19 + colorShift)));
  }

  outputColor = vec4(color * glowIntensity, 1.0);
}
