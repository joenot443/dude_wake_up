#version 150

uniform sampler2D tex;
in vec2 coord;
uniform float audio[256];
out vec4 outputColor;

void main() {
  // quantize coordinates
  const float bands = 30.0;
  const float segs = 40.0;
  vec2 p;
  p.x = floor(coord.x * bands) / bands;
  p.y = floor(coord.y * segs) / segs;

  // read frequency data from first row of texture
  float fft = audio[int(p.x * 256.)];

  // led color
  vec3 color = mix(vec3(0.0, 2.0, 0.0), vec3(2.0, 0.0, 0.0), sqrt(coord.y));

  // mask for bar graph
  float mask = (p.y < fft) ? 1.0 : 0.1;

  // led shape
  vec2 d = fract((coord - p) * vec2(bands, segs)) - 0.5;
  float led = smoothstep(0.5, 0.35, abs(d.x)) * smoothstep(0.5, 0.35, abs(d.y));
  vec3 ledColor = led * color * mask;

  // output final color
  outputColor = vec4(ledColor, 1.0);
}
