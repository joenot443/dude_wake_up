#version 150

uniform sampler2D tex;
uniform vec2 dimensions;
uniform float time;
in vec2 coord;
out vec4 outputColor;


float hash(float n) { return fract(sin(n) * 43758.5453123); }

float noise(vec2 x){
  vec2 p = floor(x);
  vec2 f = fract(x);
  float a = hash(p.x + p.y * 57.0);
  float b = hash(p.x + 1.0 + p.y * 57.0);
  float c = hash(p.x + (p.y + 1.0) * 57.0);
  float d = hash(p.x + 1.0 + (p.y + 1.0) * 57.0);
  vec2 u = f * f * (3.0 - 2.0 * f);
  return mix(a, b, u.x) +
  (c - a) * u.y * (1.0 - u.x) +
  (d - b) * u.x * u.y;
}

float highAmpNoise(vec2 x) {
  return noise(x) * 2.0;
}

float fbm(vec2 x) {
  float v = 0.0;
  float a = 0.5;
  float angle = 0.5 + time * 0.001;
  mat2 rot = mat2(cos(angle), sin(angle), -sin(angle), cos(angle));
  for (int i = 0; i < 8; ++i) {
    v += a * highAmpNoise(x);
    x = rot * x * 2.0;
    a *= 0.5;
  }
  return v;
}

float domainWarping(vec2 p) {
  float t = time * 0.0001;
  float timeStep = noise(vec2(time*0.0001, 0.0));
  float myNoiseValue = noise(vec2(timeStep, 0.0));
  float mxNoiseValue = noise(vec2(timeStep, 100.0));
  float my = mix(0.5, 2.0, myNoiseValue);
  float mx = mix(1.0, 100.0, mxNoiseValue);
  
  vec2 highAmpWarp = vec2(
                          highAmpNoise(p * 0.1 * my + t),
                          highAmpNoise(p * 0.1 * my + t + 10.0 * mx)
                          );
  
  vec2 q = vec2(fbm(p + highAmpWarp + vec2(0.0, 0.0)),
                fbm(p + highAmpWarp + vec2(5.2, 1.3)));
  vec2 r = vec2(fbm(p + 3.0 * q + vec2(1.7, 9.2 + t)),
                fbm(p + 3.0 * q + vec2(8.3 + t, 2.8)));
  vec2 s = vec2(fbm(p + 3.0 * r + vec2(3.4, 7.1 + t)),
                fbm(p + 3.0 * r + vec2(5.6 + t, 1.2)));
  return fbm(0.4 * p + 0.4 * s);
}

vec3 hexToRgb(int hexValue) {
  return vec3(
              float((hexValue >> 16) & 0xFF) / 255.0,
              float((hexValue >> 8) & 0xFF) / 255.0,
              float(hexValue & 0xFF) / 255.0
              );
}

vec3 palette(float t) {
  t = fract(t + sin(t * 123.456) * 0.05);
  t = clamp(t, 0.0, 1.0);
  
  int hexColors[5] = int[5](
                            0x2C3531, // #2C3531
                            0x116466, // #116466
                            0xD9B08C, // #D9B08C
                            0xFFCB9A, // #FFCB9A
                            0xD1E8E2  // #D1E8E2
                            );
  
  vec3 colors[5];
  for (int i = 0; i < 5; i++) {
    colors[i] = hexToRgb(hexColors[i]);
    colors[i] = mix(vec3(0.5), colors[i] * 0.9, 1.2);
  }
  float thresholds[15] = float[15](
                                   0.07, 0.15, 0.22, 0.28, 0.35, 0.42, 0.48, 0.55, 0.62, 0.68, 0.75, 0.82, 0.88, 0.95, 1.0
                                   );
  
  if (t < thresholds[0]) {
    return mix(colors[0], colors[1], t / thresholds[0]);
  } else if (t < thresholds[1]) {
    return mix(colors[1], colors[2], (t - thresholds[0]) / (thresholds[1] - thresholds[0]));
  } else if (t < thresholds[2]) {
    return mix(colors[2], colors[3], (t - thresholds[1]) / (thresholds[2] - thresholds[1]));
  } else if (t < thresholds[3]) {
    return mix(colors[3], colors[4], (t - thresholds[2]) / (thresholds[3] - thresholds[2]));
  } else if (t < thresholds[4]) {
    return mix(colors[4], colors[0], (t - thresholds[3]) / (thresholds[4] - thresholds[3]));
  } else if (t < thresholds[5]) {
    return mix(colors[0], colors[1], (t - thresholds[4]) / (thresholds[5] - thresholds[4]));
  } else if (t < thresholds[6]) {
    return mix(colors[1], colors[2], (t - thresholds[5]) / (thresholds[6] - thresholds[5]));
  } else if (t < thresholds[7]) {
    return mix(colors[2], colors[3], (t - thresholds[6]) / (thresholds[7] - thresholds[6]));
  } else if (t < thresholds[8]) {
    return mix(colors[3], colors[4], (t - thresholds[7]) / (thresholds[8] - thresholds[7]));
  } else if (t < thresholds[9]) {
    return mix(colors[4], colors[0], (t - thresholds[8]) / (thresholds[9] - thresholds[8]));
  } else if (t < thresholds[10]) {
    return mix(colors[0], colors[1], (t - thresholds[9]) / (thresholds[10] - thresholds[9]));
  } else if (t < thresholds[11]) {
    return mix(colors[1], colors[2], (t - thresholds[10]) / (thresholds[11] - thresholds[10]));
  } else if (t < thresholds[12]) {
    return mix(colors[2], colors[3], (t - thresholds[11]) / (thresholds[12] - thresholds[11]));
  } else if (t < thresholds[13]) {
    return mix(colors[3], colors[4], (t - thresholds[12]) / (thresholds[13] - thresholds[12]));
  } else {
    return mix(colors[4], colors[0], (t - thresholds[13]) / (thresholds[14] - thresholds[13]));
  }
}


void main(  )
{
  vec2 uv = coord.xy / dimensions.xy;
  uv = uv * 2.0 - 1.0;
  uv.x *= dimensions.x / dimensions.y;
  float n = domainWarping(uv * 2.0);
  vec3 color = palette(n);
  outputColor = vec4(color, 1.0);
}

