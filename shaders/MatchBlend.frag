#version 150

// MatchBlend
// Blends two sources where the per-pixel mix amount is driven by how closely
// the two sources "match" at that pixel. Closer match -> stronger blend.
// The match scalar is computed from a selectable metric (color/luma/hue/sat).

uniform sampler2D tex;   // source A (main input)
uniform sampler2D tex2;  // source B (second input)
uniform vec2 dimensions;
uniform float time;

uniform int metric;       // 0 = RGB Distance, 1 = Luminance, 2 = Hue, 3 = Saturation
uniform float threshold;  // distance at/above which match -> 0
uniform float smoothness; // softness of the match falloff (0 = hard edge)
uniform float strength;   // ceiling on the mix amount
uniform int invert;       // 1 = closer match blends LESS
uniform int showMask;     // 1 = output the raw match scalar as grayscale (for tuning)

in vec2 coord;
out vec4 outputColor;

// RGB -> HSV (h,s,v all in 0..1)
vec3 rgb2hsv(vec3 c) {
  vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
  vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
  vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));
  float d = q.x - min(q.w, q.y);
  float e = 1.0e-10;
  return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)),
              d / (q.x + e),
              q.x);
}

// Returns a normalized "distance" in 0..1, where 0 means a perfect match.
float metricDistance(vec3 a, vec3 b) {
  if (metric == 0) {
    // RGB euclidean distance, normalized by sqrt(3) (max possible distance)
    return distance(a, b) / 1.7320508;
  } else if (metric == 1) {
    // Luminance (brightness) difference
    float la = dot(a, vec3(0.299, 0.587, 0.114));
    float lb = dot(b, vec3(0.299, 0.587, 0.114));
    return abs(la - lb);
  } else if (metric == 2) {
    // Hue difference (circular), normalized to 0..1
    vec3 ha = rgb2hsv(a);
    vec3 hb = rgb2hsv(b);
    float d = abs(ha.x - hb.x);
    d = min(d, 1.0 - d) * 2.0; // shortest way around the hue wheel
    // Hue is meaningless when either pixel is nearly gray; fall back to full
    // distance so desaturated regions don't read as a spurious match.
    float satWeight = min(ha.y, hb.y);
    return mix(1.0, d, satWeight);
  } else {
    // Saturation difference
    float sa = rgb2hsv(a).y;
    float sb = rgb2hsv(b).y;
    return abs(sa - sb);
  }
}

void main() {
  vec4 a = texture(tex, coord);
  vec4 b = texture(tex2, coord);

  float d = metricDistance(a.rgb, b.rgb); // 0 = identical

  // Map distance -> match scalar (1 = close match, 0 = far apart).
  float edge1 = max(threshold, 1.0e-4);
  float edge0 = edge1 * (1.0 - clamp(smoothness, 0.0, 0.999));
  edge0 = min(edge0, edge1 - 1.0e-4);
  float match = 1.0 - smoothstep(edge0, edge1, d);

  if (invert == 1) match = 1.0 - match;

  if (showMask == 1) {
    outputColor = vec4(vec3(match), 1.0);
    return;
  }

  float m = match * strength;
  vec3 col = mix(a.rgb, b.rgb, m);
  outputColor = vec4(col, 1.0);
}
