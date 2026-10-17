#version 150

uniform sampler2D tex;
uniform vec2 dimensions;
uniform float time;
uniform float rotationAngle;
uniform float pivotU;
uniform float pivotV;
uniform float saturationGamma;
uniform float lumaShift;
uniform float mixAmount;
in vec2 coord;
out vec4 outputColor;

// BT.601 RGB <-> YCbCr (chroma centered around 0)
vec3 rgbToYCbCr(vec3 c) {
  float y  =  0.299 * c.r + 0.587 * c.g + 0.114 * c.b;
  float cb = -0.168736 * c.r - 0.331264 * c.g + 0.5 * c.b;
  float cr =  0.5 * c.r - 0.418688 * c.g - 0.081312 * c.b;
  return vec3(y, cb, cr);
}

vec3 yCbCrToRgb(vec3 c) {
  float y  = c.x;
  float cb = c.y;
  float cr = c.z;
  float r = y + 1.402 * cr;
  float g = y - 0.344136 * cb - 0.714136 * cr;
  float b = y + 1.772 * cb;
  return vec3(r, g, b);
}

void main() {
  vec2 uv = coord / dimensions.xy;
  vec4 src = texture(tex, uv);
  vec3 ycc = rgbToYCbCr(src.rgb);

  // Move chroma into the pivot frame (so the white-point is the new origin).
  vec2 chroma = ycc.yz - vec2(pivotU, pivotV);

  // Saturation lift: reshape magnitude with a gamma curve while preserving
  // direction. Distant colors become more vivid, near-pivot colors stay tame.
  float r0 = length(chroma);
  if (r0 > 1e-6) {
    float r1 = pow(r0 * 2.0, saturationGamma) * 0.5;
    chroma *= r1 / r0;
  }

  // Spin the chroma plane by rotationAngle.
  float ca = cos(rotationAngle);
  float sa = sin(rotationAngle);
  vec2 spun = vec2(
    ca * chroma.x - sa * chroma.y,
    sa * chroma.x + ca * chroma.y
  );

  // Re-anchor around the pivot.
  ycc.yz = spun + vec2(pivotU, pivotV);

  // Optional luma bias for shadow/highlight pull.
  ycc.x = clamp(ycc.x + lumaShift, 0.0, 1.0);

  vec3 graded = clamp(yCbCrToRgb(ycc), 0.0, 1.0);
  vec3 finalCol = mix(src.rgb, graded, mixAmount);
  outputColor = vec4(finalCol, src.a);
}
