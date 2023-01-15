#version 150

uniform sampler2D tex;
uniform float audio[256];
uniform float time;

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

  for (float i = 0.0; i < 5.0; i++) {

    uv.y += (0.2 * sin(uv.x + i / 7.0 - time * 0.6));
    
    float a = audio[int(coord.x * 256.)] - 0.5;
    
    float Y = uv.y + getWeight(squared(i) * 20.0) * a;
    
    lineIntensity =
        0.4 + squared(1.6 * abs(mod(coord.x + i / 1.3 + time, 2.0) - 1.0));
    glowWidth = abs(lineIntensity / (150.0 * Y));
    color += vec3(glowWidth * (2.0 + sin(time * 0.13)),
                  glowWidth * (2.0 - sin(time * 0.23)),
                  glowWidth * (2.0 - cos(time * 0.19)));
  }

  outputColor = vec4(color, 1.0);
}
