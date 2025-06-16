#version 150

// Input uniforms
uniform sampler2D tex;
uniform float time;
uniform vec2 dimensions;
uniform float cycles;
uniform float spread;
uniform float speed;
uniform float smoothness;
uniform float offset;
// Vertex shader outputs
in vec2 coord;
out vec4 outputColor;

// Converts wavelength (400-700nm) to RGB color
vec3 wavelengthToRGB(float wavelength) {
  float r = 0.0, g = 0.0, b = 0.0;
  
  // Red component calculation
  if ((wavelength >= 400.0) && (wavelength < 410.0)) {
    float t = (wavelength - 400.0) / (410.0 - 400.0);
    r = +(0.33 * t) - (0.20 * t * t);
  } else if ((wavelength >= 410.0) && (wavelength < 475.0)) {
    float t = (wavelength - 410.0) / (475.0 - 410.0);
    r = 0.14 - (0.13 * t * t);
  } else if ((wavelength >= 545.0) && (wavelength < 595.0)) {
    float t = (wavelength - 545.0) / (595.0 - 545.0);
    r = +(1.98 * t) - (t * t);
  } else if ((wavelength >= 595.0) && (wavelength < 650.0)) {
    float t = (wavelength - 595.0) / (650.0 - 595.0);
    r = 0.98 + (0.06 * t) - (0.40 * t * t);
  } else if ((wavelength >= 650.0) && (wavelength < 700.0)) {
    float t = (wavelength - 650.0) / (700.0 - 650.0);
    r = 0.65 - (0.84 * t) + (0.20 * t * t);
  }
  
  // Green component calculation
  if ((wavelength >= 415.0) && (wavelength < 475.0)) {
    float t = (wavelength - 415.0) / (475.0 - 415.0);
    g = +(0.80 * t * t);
  } else if ((wavelength >= 475.0) && (wavelength < 590.0)) {
    float t = (wavelength - 475.0) / (590.0 - 475.0);
    g = 0.8 + (0.76 * t) - (0.80 * t * t);
  } else if ((wavelength >= 585.0) && (wavelength < 639.0)) {
    float t = (wavelength - 585.0) / (639.0 - 585.0);
    g = 0.82 - (0.80 * t);
  }
  
  // Blue component calculation
  if ((wavelength >= 400.0) && (wavelength < 475.0)) {
    float t = (wavelength - 400.0) / (475.0 - 400.0);
    b = +(2.20 * t) - (1.50 * t * t);
  } else if ((wavelength >= 475.0) && (wavelength < 560.0)) {
    float t = (wavelength - 475.0) / (560.0 - 475.0);
    b = 0.7 - (t) + (0.30 * t * t);
  }
  
  return vec3(r, g, b);
}

// Maps input value to spectral color
vec3 getSpectralColor(float x) {
  return wavelengthToRGB(x * 200.0 * spread + cycles * 200.0 - (-1.0 + offset) * 200.0);
}

void main() {
  // Normalize coordinates to [-1, 1] range
  vec2 normalizedCoord = (2.0 * coord.xy - dimensions.xy) / max(dimensions.x, dimensions.y);
  
  // Apply iterative distortion
  for (int i = 1; i < 50 * cycles; i++) {
    normalizedCoord = normalizedCoord +
    vec2(smoothness / float(i) * sin(float(i) * normalizedCoord.y + time * speed + 0.3 * float(i)) + 1.0,
         smoothness / float(i) * sin(float(i) * normalizedCoord.x + time * speed + 0.3 * float(i + 10)) - 1.4);
  }
  
  // Generate final color with gamma correction
  vec3 finalColor = getSpectralColor(normalizedCoord.x - 48.5 * cycles);
  outputColor = vec4(pow(finalColor, vec3(1.0 / 2.2)), 1.0);
}
