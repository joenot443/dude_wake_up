#version 150

uniform sampler2D tex;
uniform float time;
uniform vec2 dimensions;
uniform float amount;
uniform float colorScale;
uniform float starCount;
uniform float moonPhase;
uniform float skyBrightness;
uniform float galaxyScale;
uniform float cameraSpeed;
in vec2 coord;
out vec4 outputColor;

const float NUM_LAYERS = 5.0;

const float PI = 3.141592654;
const float TAU = 2.0 * PI;
#define TIME mod(time, 30.0)
#define TTIME (TAU * TIME)
#define RESOLUTION dimensions
#define ROT(angle) mat2(cos(angle), sin(angle), -sin(angle), cos(angle))

float linearToSRGB(float linearValue) {
  return mix(1.055 * pow(linearValue, 1.0 / 2.4) - 0.055, 12.92 * linearValue, step(linearValue, 0.0031308));
}
vec3 linearToSRGB(in vec3 linearColor) {
  return vec3(linearToSRGB(linearColor.x), linearToSRGB(linearColor.y), linearToSRGB(linearColor.z));
}

vec3 acesApprox(vec3 colorValue) {
  colorValue = max(colorValue, 0.0);
  colorValue *= 0.6;
  float a = 2.51;
  float b = 0.03;
  float c = 2.43;
  float d = 0.59;
  float e = 0.14;
  return clamp((colorValue * (a * colorValue + b)) / (colorValue * (c * colorValue + d) + e), 0.0, 1.0);
}

float tanhApprox(float x) {
  float xSquared = x * x;
  return clamp(x * (27.0 + xSquared) / (27.0 + 9.0 * xSquared), -1.0, 1.0);
}

const vec4 HSV_TO_RGB_K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
vec3 hsvToRgb(vec3 hsvColor) {
  vec3 intermediate = abs(fract(hsvColor.xxx + HSV_TO_RGB_K.xyz) * 6.0 - HSV_TO_RGB_K.www);
  return hsvColor.z * mix(HSV_TO_RGB_K.xxx, clamp(intermediate - HSV_TO_RGB_K.xxx, 0.0, 1.0), hsvColor.y);
}

vec2 mod2(inout vec2 position, vec2 gridSize) {
  vec2 cellIndex = floor((position + gridSize * 0.5) / gridSize);
  position = mod(position + gridSize * 0.5, gridSize) - gridSize * 0.5;
  return cellIndex;
}

vec2 hash2(vec2 inputVec) {
  inputVec = vec2(dot(inputVec, vec2(127.1, 311.7)), dot(inputVec, vec2(269.5, 183.3)));
  return fract(sin(inputVec) * 43758.5453123);
}

vec2 signedHash2(vec2 inputVec) {
  return -1.0 + 2.0 * hash2(inputVec);
}

vec3 cartesianToSpherical(vec3 cartesianCoords) {
  float radius = length(cartesianCoords);
  float theta = acos(cartesianCoords.z / radius);
  float phi = atan(cartesianCoords.y, cartesianCoords.x);
  return vec3(radius, theta, phi);
}

vec3 blackbodyColor(float temperature) {
  vec3 color = vec3(255.0);
  color.x = 56100000.0 * pow(temperature, -1.5) + 148.0;
  color.y = 100.04 * log(temperature) - 623.6 * colorScale;
  if (temperature > 6500.0) {
    color.y = 35200000.0 * pow(temperature, -1.5) + 184.0;
  }
  color.z = 194.18 * log(temperature) - 1448.6;
  color = clamp(color, 0.0, 255.0) / 255.0;
  if (temperature < 1000.0) {
    color *= temperature / 1000.0;
  }
  return color;
}

float noise(vec2 position) {
  vec2 integerPart = floor(position);
  vec2 fractionalPart = fract(position);
  vec2 interpolant = fractionalPart * fractionalPart * (3.0 - 2.0 * fractionalPart);
  
  float noiseValue = mix(mix(dot(signedHash2(integerPart + vec2(0.0, 0.0)), fractionalPart - vec2(0.0, 0.0)),
                             dot(signedHash2(integerPart + vec2(1.0, 0.0)), fractionalPart - vec2(1.0, 0.0)), interpolant.x),
                         mix(dot(signedHash2(integerPart + vec2(0.0, 1.0)), fractionalPart - vec2(0.0, 1.0)),
                             dot(signedHash2(integerPart + vec2(1.0, 1.0)), fractionalPart - vec2(1.0, 1.0)), interpolant.x),
                         interpolant.y);
  
  return 2.0 * noiseValue;
}

float fbm(vec2 position, float offset, float scale, int iterations) {
  position *= scale;
  position += offset;
  
  const float amplitudeFactor = 0.5;
  const mat2 rotationMatrix = 2.04 * ROT(1.0);
  
  float heightSum = 0.0;
  float amplitude = 1.0;
  float amplitudeSum = 0.0;
  for (int i = 0; i < iterations; ++i) {
    amplitudeSum += amplitude;
    heightSum += amplitude * noise(position);
    position += vec2(10.7, 8.3);
    position *= rotationMatrix;
    amplitude *= amplitudeFactor;
  }
  heightSum /= amplitudeSum;
  
  return heightSum;
}

float calculateHeight(vec2 position) {
  float fbmValue = fbm(position, 0.0, 5.0, 5);
  fbmValue *= 0.3;
  return fbmValue;
}

vec3 renderStars(vec3 rayOrigin, vec3 rayDirection, vec2 sphericalCoords, float heightFactor) {
  vec3 starColorSum = vec3(0.0);
  
  const float numLayers = NUM_LAYERS;
  heightFactor = tanhApprox(20.0 * heightFactor);
  
  for (float layerIndex = 0.0; layerIndex < numLayers; ++layerIndex) {
    vec2 layerPlaneCoords = sphericalCoords + 0.5 * layerIndex;
    float layerScale = layerIndex / (numLayers - 1.0);
    vec2 cellDimensions = vec2(mix(0.05, 0.003, layerScale) * PI);
    vec2 cellCoords = mod2(layerPlaneCoords, cellDimensions);
    vec2 hashVal = hash2(cellCoords + 127.0 + layerIndex);
    vec2 offset = -1.0 + 2.0 * hashVal;
    float sinX = sin(sphericalCoords.x);
    layerPlaneCoords += offset * cellDimensions * 0.5;
    layerPlaneCoords.y *= sinX;
    float distFromCenter = length(layerPlaneCoords);
    
    float hashFract1 = fract(hashVal.x * 1667.0);
    float hashFract2 = fract(hashVal.x * 1887.0);
    float hashFract3 = fract(hashVal.x * 2997.0);
    
    vec3 singleStarColor = mix(8.0 * hashFract2, 0.25 * hashFract2 * hashFract2, layerScale) *
    blackbodyColor(mix(3000.0, 22000.0, hashFract1 * hashFract1));
    
    vec3 cumulativeColor = starColorSum + exp(-(mix(6000.0, 2000.0, heightFactor) / mix(2.0, 0.25, layerScale)) *
                                              max(distFromCenter - 0.001, 0.0)) *
    singleStarColor;
    
    starColorSum = hashFract3 < sinX ? cumulativeColor : starColorSum;
  }
  
  return starColorSum * starCount; // 1.0: starCount
}

vec2 raySphereIntersection(vec3 rayOrigin, vec3 rayDirection, vec4 sphere) {
  vec3 originToCenter = rayOrigin - sphere.xyz;
  float b = dot(originToCenter, rayDirection);
  float c = dot(originToCenter, originToCenter) - sphere.w * sphere.w;
  float discriminant = b * b - c;
  if (discriminant < 0.0) {
    return vec2(-1.0);
  }
  discriminant = sqrt(discriminant);
  return vec2(-b - discriminant, -b + discriminant);
}

vec4 renderMoon(vec3 rayOrigin, vec3 rayDirection, vec2 sphericalCoords, vec3 lp, vec4 md) {
  vec2 mi = raySphereIntersection(rayOrigin, rayDirection, md);
  
  vec3 p = rayOrigin + mi.x * rayDirection;
  vec3 n = normalize(p - md.xyz);
  vec3 r = reflect(rayDirection, n) * 1000. * moonPhase; // 0.001: moonPhase
  vec3 ld = normalize(lp - p);
  float fre = dot(n, rayDirection) + 1.0;
  fre = pow(fre, 15.0);
  float dif = max(dot(ld, n), 0.0);
  float spe = pow(max(dot(ld, r), 0.0), 8.0);
  float i = 0.5 * tanhApprox(20.0 * fre * spe + 0.05 * dif);
  vec3 col = blackbodyColor(1500.0) * i + hsvToRgb(vec3(0.6, mix(0.6, 0.0, i), i));
  
  float t = tanhApprox(0.25 * (mi.y - mi.x));
  
  return vec4(vec3(col), t);
}

vec3 renderSky(vec3 rayOrigin, vec3 rayDirection, vec2 sphericalCoords, vec3 lp, out float cf) {
  float ld = max(dot(normalize(lp - rayOrigin), rayDirection), 0.0);
  float y = -0.5 + sphericalCoords.x / PI;
  y = max(abs(y) - 0.02, 0.0) + 0.1 * smoothstep(0.5, PI, abs(sphericalCoords.y));
  vec3 blue = hsvToRgb(vec3(0.6, 0.75, 0.35 * exp(-15.0 * y)));
  float ci = pow(ld, 10.0) * 2.0 * exp(-25.0 * y) * skyBrightness; // 1.0: skyBrightness
  vec3 yellow = blackbodyColor(1500.0) * ci;
  cf = ci;
  return blue + yellow;
}

vec3 renderGalaxy(vec3 rayOrigin, vec3 rayDirection, vec2 sphericalCoords, out float sf) {
  vec2 gp = sphericalCoords;
  gp *= ROT(0.67);
  gp += vec2(-1.0, 0.5);
  float h1 = calculateHeight(2.0 * sphericalCoords) * galaxyScale; // 1.0: galaxyScale
  float gcc = dot(gp, gp);
  float gcx = exp(-(abs(3.0 * (gp.x))));
  float gcy = exp(-abs(10.0 * (gp.y)));
  float gh = gcy * gcx;
  float cf = smoothstep(0.05, -0.2, -h1);
  vec3 col = vec3(0.0);
  col += blackbodyColor(mix(300.0, 1500.0, gcx * gcy)) * gcy * gcx;
  col += hsvToRgb(vec3(0.6, 0.5, 0.00125 / gcc));
  col *= mix(mix(0.15, 1.0, gcy * gcx), 1.0, cf);
  sf = gh * cf;
  return col;
}

vec3 renderGrid(vec3 rayOrigin, vec3 rayDirection, vec2 sphericalCoords) {
  const float m = 1.0;
  
  const vec2 dim = vec2(1.0 / 8.0 * PI);
  vec2 pp = sphericalCoords;
  vec2 np = mod2(pp, dim);
  
  vec3 col = vec3(0.0);
  
  float y = sin(sphericalCoords.x);
  float d = min(abs(pp.x), abs(pp.y * y));
  
  float aa = 2.0 / RESOLUTION.y;
  
  col += 2.0 * vec3(0.5, 0.5, 1.0) * exp(-2000.0 * max(d - 0.00025, 0.0));
  
  return 0.25 * tanh(col);
}

vec3 calculatePixelColor(vec3 rayOrigin, vec3 rayDirection, vec3 lp, vec4 md) {
  vec2 sp = cartesianToSpherical(rayDirection.xzy).yz;
  
  float sf = 0.0;
  float cf = 0.0;
  vec3 col = vec3(0.0);
  
  vec4 mcol = renderMoon(rayOrigin, rayDirection, sp, lp, md);
  
  col += renderStars(rayOrigin, rayDirection, sp, sf) * (1.0 - tanhApprox(2.0 * cf));
  col += renderGalaxy(rayOrigin, rayDirection, sp, sf);
  col = mix(col, mcol.xyz, mcol.w);
  col += renderSky(rayOrigin, rayDirection, sp, lp, cf);
  col += renderGrid(rayOrigin, rayDirection, sp);
  
  if (rayDirection.y < 0.0) {
    col = vec3(0.0);
  }
  
  return col;
}

void main() {
  vec2 q = coord;
  vec2 p = -1.0 + 2.0 * q;
  p.x *= RESOLUTION.x / RESOLUTION.y;
  
  vec3 ro = vec3(0.0, 0.0, 0.0);
  vec3 lp = 500.0 * vec3(1.0, -0.25, 0.0);
  vec4 md = 50.0 * vec4(vec3(1.0, 1., -0.6), 0.5);
  vec3 la = vec3(1.0, 0.5, 0.0);
  vec3 up = vec3(0.0, 1.0, 0.0);
  la.xz *= ROT(TTIME / 60.0 * cameraSpeed - PI / 2.0);
  
  vec3 ww = normalize(la - ro);
  vec3 uu = normalize(cross(up, ww));
  vec3 vv = normalize(cross(ww, uu));
  vec3 rd = normalize(p.x * uu + p.y * vv + 2.0 * ww);
  vec3 col = calculatePixelColor(ro, rd, lp, md);
  
  col *= smoothstep(0.0, 4.0, TIME) * smoothstep(30.0, 26.0, TIME);
  col = acesApprox(col);
  col = linearToSRGB(col);
  
  outputColor = vec4(col, 1.0);
}
