#version 150

uniform sampler2D tex;
uniform vec2 dimensions;
uniform float time;
uniform float weight;
uniform vec4 strokeColor;
uniform float fineness;
uniform float edgeSoftness;
in vec2 coord;
out vec4 outputColor;

// Gaussian weights for 3x3 kernel
const float gaussian[9] = float[](
    0.0625, 0.125, 0.0625,
    0.125,  0.25,  0.125,
    0.0625, 0.125, 0.0625
);

void main() {

  const float TAU = 6.28318530;
  float steps = fineness;

  float radius = weight;
  vec2 uv = coord / dimensions.xy;

  // Correct aspect ratio
  vec2 aspect = 1.0 / vec2(textureSize(tex, 0));

  // Multi-sample the center pixel for antialiasing
  vec4 centerColor = vec4(0.0);
  float totalWeight = 0.0;

  // 3x3 supersampling for smoother text edges
  for (int sx = -1; sx <= 1; sx++) {
    for (int sy = -1; sy <= 1; sy++) {
      vec2 sampleOffset = vec2(sx, sy) * aspect * 0.5; // Half pixel sampling
      int idx = (sy + 1) * 3 + (sx + 1);
      float w = gaussian[idx];
      centerColor += texture(tex, uv + sampleOffset) * w;
      totalWeight += w;
    }
  }
  centerColor /= totalWeight;

  // Check if any text exists in the surrounding area with distance weighting
  float strokeAlpha = 0.0;
  float innerRadius = max(0.0, radius - 1.0);
  float outerRadius = radius + 1.0;

  for (float i = 0.0; i < TAU; i += TAU / steps) {
    // Sample at multiple distances for smoother falloff
    for (float r = innerRadius; r <= outerRadius; r += 0.5) {
      vec2 offset = vec2(sin(i), cos(i)) * aspect * r;
      vec4 col = texture(tex, uv + offset);

      // Distance-based weight for smoother stroke edges
      float distWeight = 1.0 - smoothstep(innerRadius, outerRadius, r);
      strokeAlpha = max(strokeAlpha, col.a * distWeight);
    }
  }

  // Smooth stroke factor with better edge transitions using uniform parameter
  float strokeFactor = smoothstep(0.0, edgeSoftness, (1.0 - centerColor.a) * strokeAlpha);

  // Apply stroke with proper alpha blending
  vec4 strokeLayer = vec4(strokeColor.rgb, strokeColor.a * strokeFactor);

  // Composite: stroke behind, text on top
  vec4 result;
  if (centerColor.a > 0.01) {
    // Text pixel: blend text over stroke (in case of semi-transparent text)
    float bgStroke = strokeAlpha * (1.0 - centerColor.a);
    result = mix(strokeLayer, centerColor, centerColor.a);

    // Ensure we maintain antialiased edges
    result.a = max(centerColor.a, strokeLayer.a);
  } else {
    // Background pixel: just use stroke if present
    result = strokeLayer;
  }

  // Apply a final smoothing pass for antialiased output
  outputColor = result;

  // Enhance edge antialiasing with alpha correction
  if (outputColor.a > 0.0 && outputColor.a < 1.0) {
    outputColor.a = smoothstep(0.0, 1.0, outputColor.a * 1.2);
  }
}
