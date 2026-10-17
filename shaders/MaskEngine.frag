#version 150

// MaskEngine
// Unified mask/blend shader. Produces a per-pixel mask scalar m in [0,1] from a
// selectable Source Mode, runs it through a soft falloff, then either cuts out
// (Mask) or blends two sources (Blend). Subsumes Luma, Color Key and Match Blend.

uniform sampler2D tex;   // input A (main)
uniform sampler2D tex2;  // input B (second)
uniform vec2 dimensions;
uniform float time;

uniform int sourceMode;  // 0 = Luma Range, 1 = Color Key, 2 = Source Match
uniform int outputMode;  // 0 = Mask, 1 = Blend
uniform int metric;      // interpreted per sourceMode

uniform float lower;     // Luma Range band
uniform float upper;     // Luma Range band
uniform vec4 keyColor;   // Color Key target
uniform float tolerance; // Color Key cutoff
uniform float threshold; // Source Match cutoff

uniform float softness;  // feather width (value units for luma, distance units otherwise)
uniform float strength;  // ceiling on the mask scalar
uniform int invert;
uniform int showMask;
uniform int swapInputs;

uniform int matchedSource;   // 0 = Input A, 1 = Input B, 2 = Transparent
uniform int unmatchedSource; // 0 = Input A, 1 = Input B, 2 = Transparent

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

const mat4 RGBtoYUV = mat4(0.257,  0.439, -0.148, 0.0,
                           0.504, -0.368, -0.291, 0.0,
                           0.098, -0.071,  0.439, 0.0,
                           0.0625, 0.500,  0.500, 1.0);

// Hue difference, weighted down when either pixel is near-gray (hue is unstable there).
float hueDistance(vec3 a, vec3 b) {
  vec3 ha = rgb2hsv(a);
  vec3 hb = rgb2hsv(b);
  float d = abs(ha.x - hb.x);
  d = min(d, 1.0 - d) * 2.0;
  return mix(1.0, d, min(ha.y, hb.y));
}

// Luma Range: which 1-D channel to threshold.
float channelValue(vec3 c, int m) {
  if (m == 0) return rgb2hsv(c).z;      // Brightness
  else if (m == 1) return rgb2hsv(c).y; // Saturation
  else if (m == 2) return c.r;
  else if (m == 3) return c.g;
  return c.b;
}

// Color Key: distance from a target color.
float colorDistance(vec3 a, vec3 b, int m) {
  if (m == 0) {
    vec4 ya = RGBtoYUV * vec4(a, 1.0);
    vec4 yb = RGBtoYUV * vec4(b, 1.0);
    return sqrt(pow(ya.g - yb.g, 2.0) + pow(ya.b - yb.b, 2.0)); // YUV chroma
  } else if (m == 1) {
    return distance(a, b) / 1.7320508; // RGB
  } else if (m == 2) {
    return hueDistance(a, b);
  }
  return abs(rgb2hsv(a).y - rgb2hsv(b).y); // Saturation
}

// Source Match: distance between two sources.
float matchDistance(vec3 a, vec3 b, int m) {
  if (m == 0) {
    return distance(a, b) / 1.7320508; // RGB
  } else if (m == 1) {
    float la = dot(a, vec3(0.299, 0.587, 0.114));
    float lb = dot(b, vec3(0.299, 0.587, 0.114));
    return abs(la - lb); // Luminance
  } else if (m == 2) {
    return hueDistance(a, b);
  }
  return abs(rgb2hsv(a).y - rgb2hsv(b).y); // Saturation
}

// Soft membership of a value within [lo, hi], feathered by soft on each edge.
float softBand(float v, float lo, float hi, float soft) {
  float s = max(soft, 1.0e-4);
  return smoothstep(lo - s, lo, v) * (1.0 - smoothstep(hi, hi + s, v));
}

// Soft closeness: 1 when d << thr, ramping to 0 as d crosses thr.
float softClose(float d, float thr, float soft) {
  float edge1 = max(thr, 1.0e-4);
  float edge0 = min(edge1 - max(soft, 1.0e-4), edge1 - 1.0e-4);
  return 1.0 - smoothstep(edge0, edge1, d);
}

vec4 pick(int which, vec4 a, vec4 b) {
  if (which == 0) return a;
  if (which == 1) return b;
  return vec4(0.0);
}

void main() {
  vec4 a = texture(tex, coord);
  vec4 b = texture(tex2, coord);
  if (swapInputs == 1) { vec4 t = a; a = b; b = t; }

  float m;
  if (sourceMode == 0) {
    m = softBand(channelValue(a.rgb, metric), lower, upper, softness);
  } else if (sourceMode == 1) {
    m = softClose(colorDistance(a.rgb, keyColor.rgb, metric), tolerance, softness);
  } else {
    m = softClose(matchDistance(a.rgb, b.rgb, metric), threshold, softness);
  }

  if (invert == 1) m = 1.0 - m;
  m = clamp(m, 0.0, 1.0);

  if (showMask == 1) {
    outputColor = vec4(vec3(m), 1.0);
    return;
  }

  m *= strength;

  if (outputMode == 1) {
    // Blend: mix A -> B by the mask scalar.
    outputColor = vec4(mix(a.rgb, b.rgb, m), 1.0);
  } else {
    // Mask: composite the matched source over the unmatched source by m.
    outputColor = mix(pick(unmatchedSource, a, b), pick(matchedSource, a, b), m);
  }
}
