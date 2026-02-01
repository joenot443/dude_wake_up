#version 150

uniform sampler2D tex;
uniform sampler2D tex2;
uniform int mode;
uniform int blendWithEmpty;
uniform int flip;
uniform float alpha;
uniform float amount;
uniform int alphaSwap;

in vec2 coord;
out vec4 outputColor;

// Function to implement various blending modes
vec4 blend(vec4 a, vec4 b, int mode) {
  vec4 result;
  if (mode == 0) { // Mix
    result = mix(a, b, amount);
  } else if (mode == 1) { // Multiply
    result = a * b;
  } else if (mode == 2) { // Screen
    result = 1.0 - (1.0 - a) * (1.0 - b);
  } else if (mode == 3) { // Darken
    result = min(a, b);
  } else if (mode == 4) { // Lighten
    result = max(a, b);
  } else if (mode == 5) { // Difference
    result = abs(a - b);
  } else if (mode == 6) { // Exclusion
    result = a + b - 2.0 * a * b;
  } else if (mode == 7) { // Overlay
    result = a.r < 0.5 ? (2.0 * a * b) : (1.0 - 2.0 * (1.0 - a) * (1.0 - b));
  } else if (mode == 8) { // Hard light
    result = b.r < 0.5 ? (2.0 * a * b) : (1.0 - 2.0 * (1.0 - a) * (1.0 - b));
  } else if (mode == 9) { // Soft light
    result = b.r < 0.5 ? (2.0 * a * b + a * a * (1.0 - 2.0 * b)) : (sqrt(a) * (2.0 * b - 1.0) + (2.0 * a) * (1.0 - b));
  } else if (mode == 10) { // Color dodge
    result = a / (1.0 - b);
  } else if (mode == 11) { // Linear dodge (Add)
    result = a + b;
  } else if (mode == 12) { // Burn
    result = 1.0 - (1.0 - a) / b;
  } else if (mode == 13) { // Linear burn
    result = a + b - 1.0;
  } else {
    result = a; // Default fallback
  }
  result.a = 1.0; // Ensure the output is fully opaque
  return result;
}

void main() {
  vec4 tex_color = texture(tex, coord);
  vec4 tex2_color = texture(tex2, coord);
  
  // If either texture is fully transparent, return the other
  if (tex_color.a < 0.1) {
    outputColor = tex2_color;
    return;
  }
  if (tex2_color.a < 0.1) {
    outputColor = tex_color;
    return;
  }
  // If our alpha is low, just return our tex1
  if (alpha < 0.03) {
    outputColor = tex_color;
    return;
  }
  
  vec4 result = blend(tex_color, tex2_color, mode);
  // Mix
  if (mode == 0) {
    outputColor = result;
    return;
  }
	outputColor = alphaSwap == 1 ? mix(result, tex_color, alpha) : mix(tex_color, result, alpha);
}
