#version 150

// Input uniforms
uniform sampler2D tex;           // Input texture
uniform vec2 dimensions;         // Screen dimensions
uniform float time;             // Global time

// Core parameters
uniform float planeDistance;    // Distance between planes
uniform float maxPlanes;        // Number of rendered planes
uniform float fadeStart;        // Where planes start fading
uniform float alphaCutoff;      // Transparency threshold

// Effect parameters
uniform float octaveScale;      // Scale factor between noise octaves
uniform float freqScale;        // Frequency scaling for noise
uniform float channelHeight;    // Height of sun channels
uniform float sunScale;         // Overall scale of sun effect

// Input/output variables
in vec2 coord;                  // Texture coordinates
out vec4 outputColor;           // Final output color

// Constants
const float PI = 3.141592654;
const vec4 HSV2RGB_K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
// These constants are now controlled via uniforms
// Default values shown below:
// const float PLANE_DISTANCE = 1.0 - 0.5;  // Now planeDistance uniform
// const int MAX_PLANES = 24;               // Now maxPlanes uniform
// const int FADE_START = 22;               // Now fadeStart uniform
// const float ALPHA_CUTOFF = 0.95;         // Now alphaCutoff uniform

// Color conversion functions
vec3 hsv2rgb(vec3 c) {
  vec3 p = abs(fract(c.xxx + HSV2RGB_K.xyz) * 6.0 - HSV2RGB_K.www);
  return c.z * mix(HSV2RGB_K.xxx, clamp(p - HSV2RGB_K.xxx, 0.0, 1.0), c.y);
}

// Blending functions
vec4 alphaBlend(vec4 back, vec4 front) {
  float w = front.w + back.w * (1.0 - front.w);
  vec3 xyz = (front.xyz * front.w + back.xyz * back.w * (1.0 - front.w)) / w;
  return w > 0.0 ? vec4(xyz, w) : vec4(0.0);
}

vec3 alphaBlend(vec3 back, vec4 front) {
  return mix(back, front.xyz, front.w);
}

// Utility functions
float mod1(inout float p, float size) {
  float halfsize = size * 0.5;
  float c = floor((p + halfsize) / size);
  p = mod(p + halfsize, size) - halfsize;
  return c;
}

float planex(vec2 p, float width) {
  return abs(p.y) - width;
}

float circle(vec2 p, float radius) {
  return length(p) - radius;
}

float pmin(float a, float b, float k) {
  float h = clamp(0.5 + 0.5 * (b - a) / k, 0.0, 1.0);
  return mix(b, a, h) - k * h * (1.0 - h);
}

float pmax(float a, float b, float k) {
  return -pmin(-a, -b, k);
}

float tanh_approx(float x) {
  float x2 = x * x;
  return clamp(x * (27.0 + x2) / (27.0 + 9.0 * x2), -1.0, 1.0);
}

// Hash functions for noise
float hash(float x) {
  return fract(sin(x * 12.9898) * 13758.5453);
}

float hash(vec2 p) {
  float dot_product = dot(p, vec2(127.1, 311.7));
  return fract(sin(dot_product) * 43758.5453123);
}

// Post-processing
vec3 postProcess(vec3 color, vec2 uv) {
  color = clamp(color, 0.0, 1.0);
  color = pow(color, vec3(1.0 / 2.2));  // Gamma correction
  color = color * 0.6 + 0.4 * color * color * (3.0 - 2.0 * color);
  color = mix(color, vec3(dot(color, vec3(0.33))), -0.4);
  color *= 0.5 + 0.5 * pow(19.0 * uv.x * uv.y * (1.0 - uv.x) * (1.0 - uv.y), 0.7);
  return color;
}

// Noise functions
float valueNoise(vec2 p) {
  vec2 i = floor(p);
  vec2 f = fract(p);
  vec2 u = f * f * (3.0 - 2.0 * f);

  float a = hash(i + vec2(0.0, 0.0));
  float b = hash(i + vec2(1.0, 0.0));
  float c = hash(i + vec2(0.0, 1.0));
  float d = hash(i + vec2(1.0, 1.0));
  
  float mix1 = mix(a, b, u.x);
  float mix2 = mix(c, d, u.x);
  
  return mix(mix1, mix2, u.y);
}

float fbm(vec2 p) {
  float sum = 0.0;
  float amplitude = 1.0;
  
  for (int i = 0; i < 3; ++i) {
    sum += amplitude * valueNoise(p);
    amplitude *= octaveScale;  // Control noise detail falloff
    p *= freqScale;           // Control noise frequency scaling
  }
  return sum;
}

float height(vec2 p) {
  return fbm(p) * smoothstep(0.0, 1.25 + 0.25 * sin(0.5 * p.y), abs(p.x)) - 0.35;
}

// Camera movement
vec3 offset(float z) {
  float angle = z;
  vec2 p = -0.05 * (vec2(cos(angle), sin(angle * sqrt(2.0))) + 
                    vec2(cos(angle * sqrt(0.75)), sin(angle * sqrt(0.5))));
  return vec3(p, z);
}

vec3 doffset(float z) {
  const float EPSILON = 0.1;
  return 0.5 * (offset(z + EPSILON) - offset(z - EPSILON)) / EPSILON;
}

vec3 ddoffset(float z) {
  const float EPSILON = 0.1;
  return 0.5 * (doffset(z + EPSILON) - doffset(z - EPSILON)) / EPSILON;
}

// Plane rendering
vec4 plane(vec3 rayOrigin, vec3 rayDir, vec3 planePos, vec3 offset, float antiAlias, float noise) {
  float h = hash(noise);
  float scale = mix(0.05, 0.25, h);
  vec2 p = (planePos - offset * 2.0 * vec3(1.0, 1.0, 0.0)).xy;
  float heightVal = height(vec2(p.x, planePos.z));
  float d = p.y - heightVal;
  float t = smoothstep(antiAlias, -antiAlias, d);
  
  vec3 hsv = vec3(
    fract(0.7 + 0.125 * sin(0.6 * planePos.z)),  // Hue
    0.5,                                          // Saturation
    smoothstep(antiAlias, -antiAlias, abs(d) - antiAlias)  // Value
  );
  
  float glow = exp(-90.0 * max(abs(d), 0.0));
  hsv.z += glow;
  hsv.z += (heightVal * heightVal - planePos.y - 0.125) * 0.5;
  vec3 color = hsv2rgb(hsv);
  
  return vec4(color, tanh_approx(t + glow));
}

// Sun shape
float sun(vec2 p) {
  vec2 sunPos = p;
  vec2 channelPos = p;
  mod1(channelPos.y, channelHeight * 6.0);
  
  float circle_d = circle(sunPos, 0.5);
  float plane_d = planex(channelPos, channelHeight);
  float clip_d = p.y + channelHeight * 3.0;
  
  float d = circle_d;
  d = pmax(d, -max(plane_d, clip_d), channelHeight * 2.0);
  return d;
}

float distanceField(vec2 p) {
  float sunDist = sun(p / sunScale) * sunScale;
  return sunDist;
}

// Sky rendering
vec3 skyColor(vec3 rayOrigin, vec3 rayDir) {
  float antiAlias = 2.0 / dimensions.y;
  vec2 p = rayDir.xy * 2.0;
  p.y -= 0.25;
  
  vec3 sunColor = mix(
    vec3(1.0, 1.0, 0.0),
    vec3(1.0, 0.0, 1.0),
    clamp((0.85 - p.y) * 0.75, 0.0, 1.0)
  );
  
  vec3 glareColor = sqrt(sunColor);
  float skyShade = smoothstep(-1.05, 0.0, p.y);
  vec3 glowColor = mix(vec3(1.0, 0.7, 0.6).zyx, glareColor, skyShade);
 
  float scale = 15.0;
  float dist = distanceField(p * scale) / scale;
  float borderDist = abs(dist) - 0.0025;
 
  vec3 color = vec3(1.0, 0.0, 1.0) * 0.125;
  vec3 corona = 0.65 * glowColor * exp(-2.5 * dist) * skyShade;
  color += corona;
  color = mix(color, sunColor * skyShade, smoothstep(-antiAlias, antiAlias, -dist));
  color = mix(color, glowColor * 1.55, smoothstep(-antiAlias, antiAlias, -borderDist));

  return color;
}

// Main scene color calculation
vec3 calculateColor(vec3 ww, vec3 uu, vec3 vv, vec3 rayOrigin, vec2 p) {
  float lenP = length(p);
  vec2 nextP = p + 1.0 / dimensions.xy;
  float rayDistMult = 2.0;
  
  vec3 rayDir = normalize(p.x * uu + p.y * vv + rayDistMult * ww);
  vec3 nextRayDir = normalize(nextP.x * uu + nextP.y * vv + rayDistMult * ww);

  float nz = floor(rayOrigin.z / planeDistance);
  float fadeDist = planeDistance * (maxPlanes - fadeStart);
  
  vec3 sky = skyColor(rayOrigin, rayDir);
  vec4 accumColor = vec4(0.0);
  bool cutOut = false;

  // Render planes from nearest to furthest
  for (int i = 1; i <= int(maxPlanes); ++i) {
    float planeZ = planeDistance * nz + planeDistance * float(i);
    float planeDist = (planeZ - rayOrigin.z) / rayDir.z;
    vec3 planePos = rayOrigin + rayDir * planeDist;
    
    if (planePos.y < 1.25 && planeDist > 0.0 && accumColor.w < alphaCutoff) {
      vec3 nextPlanePos = rayOrigin + nextRayDir * planeDist;
      float antiAlias = 3.0 * length(planePos - nextPlanePos);
      vec3 off = offset(planePos.z);
      
      vec4 planeColor = plane(rayOrigin, rayDir, planePos, off, antiAlias, nz + float(i));
      
      float zDist = planePos.z - rayOrigin.z;
      float fadeIn = smoothstep(planeDistance * maxPlanes, planeDistance * fadeStart, zDist);
      float fadeOut = smoothstep(0.0, planeDistance * 0.1, zDist);
      
      planeColor.xyz = mix(sky, planeColor.xyz, fadeIn);
      planeColor.w *= fadeOut;
      planeColor = clamp(planeColor, 0.0, 1.0);
      
      accumColor = alphaBlend(planeColor, accumColor);
    } else {
      cutOut = true;
      accumColor.w = accumColor.w > alphaCutoff ? 1.0 : accumColor.w;
      break;
    }
  }
  
  return alphaBlend(sky, accumColor);
}

// Main visual effect
vec3 effect(vec2 p, vec2 uv) {
  float timeMultiplied = time * 0.25;
  vec3 rayOrigin = offset(timeMultiplied);
  vec3 dRayOrigin = doffset(timeMultiplied);
  vec3 ddRayOrigin = ddoffset(timeMultiplied);

  vec3 ww = normalize(dRayOrigin);
  vec3 uu = normalize(cross(normalize(vec3(0.0, 1.0, 0.0) + ddRayOrigin), ww));
  vec3 vv = normalize(cross(ww, uu));

  return calculateColor(ww, uu, vv, rayOrigin, p);
}

void main() {
  vec2 uv = coord / dimensions.xy;
  uv.y = 1.0 - uv.y;
  vec2 p = -1.0 + 2.0 * uv;
  p.x *= dimensions.x / dimensions.y;
  
  vec3 color = effect(p, uv);
  color *= smoothstep(0.0, 4.0, time);
  color = postProcess(color, uv);
  
  outputColor = vec4(color, 1.0);
}