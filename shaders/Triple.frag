#version 150

uniform sampler2D tex;
uniform vec2 dimensions;
uniform float time;
uniform float xShift;
uniform float yShift;
uniform float scale;
uniform int blendMode;
uniform int drawLeft;
uniform int drawCenter;
uniform int drawRight;
in vec2 coord;
out vec4 outputColor;

// Function to implement various blending modes
vec4 blend(vec4 a, vec4 b, int mode) {
  vec4 result;
  if (mode == 0) { // Multiply
    result = a * b;
  } else if (mode == 1) { // Screen
    result = 1.0 - (1.0 - a) * (1.0 - b);
  } else if (mode == 2) { // Darken
    result = min(a, b);
  } else if (mode == 3) { // Lighten
    result = max(a, b);
  } else if (mode == 4) { // Difference
    result = abs(a - b);
  } else if (mode == 5) { // Exclusion
    result = a + b - 2.0 * a * b;
  } else if (mode == 6) { // Overlay
    result = a.r < 0.5 ? (2.0 * a * b) : (1.0 - 2.0 * (1.0 - a) * (1.0 - b));
  } else if (mode == 7) { // Hard light
    result = b.r < 0.5 ? (2.0 * a * b) : (1.0 - 2.0 * (1.0 - a) * (1.0 - b));
  } else if (mode == 8) { // Soft light
    result = b.r < 0.5 ? (2.0 * a * b + a * a * (1.0 - 2.0 * b)) : (sqrt(a) * (2.0 * b - 1.0) + (2.0 * a) * (1.0 - b));
  } else if (mode == 9) { // Color dodge
    result = a / (1.0 - b);
  } else if (mode == 10) { // Linear dodge (Add)
    result = a + b;
  } else if (mode == 11) { // Burn
    result = 1.0 - (1.0 - a) / b;
  } else if (mode == 12) { // Linear burn
    result = a + b - 1.0;
  } else {
    result = a; // Default fallback
  }
  result.a = 1.0; // Ensure the output is fully opaque
  return result;
}
void main()
{
    // Convert screen dimensions to UV space
    vec2 uvDimensions = vec2(1.0, dimensions.y / dimensions.x);

    // Center for scaling
    vec2 center = vec2(0.5, 0.5);

    // Calculate scaled and shifted coordinates for three textures
    vec2 scaledCoord = (coord - center) * scale + center; // Scale around the center
    vec2 coords[3];
    coords[0] = scaledCoord + vec2(-xShift, yShift);  // Left texture shifts left and up
    coords[1] = scaledCoord;                          // Center texture remains centered
    coords[2] = scaledCoord + vec2(xShift, -yShift);  // Right texture shifts right and down

    // Initialize color sum and count for averaging
    vec4 colorSum = vec4(0.0);
    float count = 0.0;

    // Sum colors and count only for non-transparent pixels and if drawing is enabled
    int drawFlags[3];
    drawFlags[0] = drawLeft;
    drawFlags[1] = drawCenter;
    drawFlags[2] = drawRight;

    for (int i = 0; i < 3; i++) {
        if (drawFlags[i] == 1) {
            vec4 texSample = texture(tex, coords[i]);
            if (texSample.a > 0.0) {
                colorSum = blend(texSample, colorSum, blendMode);
                count += 1.0;
            }
        }
    }

    // Handle the case when count is zero
    if (count == 0.0) {
        outputColor = vec4(0.0);
    } else {
        outputColor = vec4(colorSum.rgb, 1.0);
    }
}



