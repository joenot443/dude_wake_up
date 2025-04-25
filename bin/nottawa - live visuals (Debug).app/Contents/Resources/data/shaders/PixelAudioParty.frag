#version 150

uniform sampler2D tex;
uniform vec2 dimensions;
uniform float amount;
uniform float audio[256];
uniform float time;
in vec2 coord;
out vec4 outputColor;


vec3 rgb2hsv(vec3 c) {
  
  vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
  
  vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
  
  vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));
  
  float d = q.x - min(q.w, q.y);
  
  float e = 1.0e-10;
  
  return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
}


vec3 hsv2rgb(vec3 c) {
  
  vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
  
  vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
  
  return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

void main() {
  float audioData = 0.0;
  vec2 uv = coord.xy / dimensions.xy;

  for (int i = 0; i < 256; i++) {
    audioData += audio[int(uv.x * 256.)] * amount;}
  
  audioData /= 256.0;
  
  float pixelsize = smoothstep(-1.0, 1.0, audioData) * audioData * 80.0;
  
  if (audioData <= 0.2) {audioData = 0.0;}
  
  if (audioData <= 0.3) {pixelsize = 1.0;}
  
  vec2 gridSize = vec2(pixelsize, pixelsize);
  
  outputColor = vec4(0.0, 0.0, 0.0, 1.0);
  
  vec2 gridPos = floor(coord / gridSize) * gridSize;
  
  vec2 webcamCoord = (gridPos + 0.5 * gridSize) / dimensions.xy;
  
  vec3 webcamColor = texture(tex, coord).rgb;
  
  vec3 hsvColor = rgb2hsv(webcamColor);
  
  hsvColor.x += audioData * 11.5;
  
  vec3 finalColor = hsv2rgb(hsvColor);
  
//  finalColor *= audioData;
  
  outputColor = vec4(finalColor, 1.0);
}
