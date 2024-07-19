#version 150

uniform sampler2D mainTexture;
uniform sampler2D fbTexture;
uniform sampler2D overlayTexture;

in vec2 coord;
out vec4 outputColor;

uniform float mainAlpha;
uniform float fbMix;
uniform float fbAlpha;

uniform int blendMode;
uniform int fbType;
uniform int priority;

uniform float scale;
uniform float rotate;
uniform vec2 translate;
uniform int lumaEnabled;
uniform int overlayEnabled;
uniform float lumaKey;
uniform float lumaThresh;
uniform vec3 hsb;
uniform vec3 rescale;
uniform vec3 invert;


/*
 
 mainAlpha: Alpha for mainTexture
 fbAlpha: Alpha for fbTexture
 fbMix: Mix for the fb/main
 
 */

// Function to implement various blending modes
vec4 blend(vec4 a, vec4 b, int mode) {
  vec4 result;
  if (mode == 0) { // Standard

    // Empty main
    if (a.a < 0.05) {
      return vec4(b.rgb, max(b.a - 0.4 * (1.0 - fbAlpha), 0.0));
    }
    
    // Empty feedback
    if (b.a < 0.05) {
      return vec4(a.rgb, mainAlpha);
    }
    
    vec4 main = vec4(a.rgb, mainAlpha);
    vec4 fb = vec4(b.rgb, fbAlpha);
    
    return mix(main, fb, fbMix);
  }
  
  else if (mode == 1) { // Multiply
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

vec3 rgb2hsb(in vec3 c) {
  vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
  vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
  vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));
  float d = q.x - min(q.w, q.y);
  float e = 1.0e-10;
  return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
}

vec3 hsb2rgb(in vec3 c) {
  vec3 rgb = clamp(abs(mod(c.x * 6.0 + vec3(0.0, 4.0, 2.0), 6.0) - 3.0) - 1.0,
                   0.0, 1.0);
  rgb = rgb * rgb * (3.0 - 2.0 * rgb);
  return c.z * mix(vec3(1.0), rgb, c.y);
}

vec4 mixLumaKey(in vec4 mainColor, in vec4 fbColor, in float key,
                in float thresh) {
  vec3 mainColorHsb = rgb2hsb(mainColor.rgb);
  vec3 fbColorHsb = rgb2hsb(fbColor.rgb);

  // Upper and lower bounds of our passing luma
  float upper = key + thresh;
  float lower = key - thresh;

  // Feedback's luma value
  float fbLuma = fbColorHsb.z;

  // Check if we're within bounds
  if (fbLuma > lower && fbLuma < upper) {
    return mix(mainColor, fbColor, fbMix);
  }

  return vec4(0.0);
}

vec2 rotate2D(in vec2 pos, in float a) {
  float cosPivot = cos(a);
  float sinPivot = sin(a);
  vec2 c = cosPivot * pos + sinPivot * vec2(pos.y, -pos.x);
  return c;
}

void main() {
  vec2 uv = coord;
  // Scale and rotate if necessary
  if (abs(1.0 - scale) > 0.01 || abs(1.0 - rotate) > 0.01) {
    vec2 center = vec2(0.5, 0.5); // Center of the texture

    // Translate the UV coordinates to center the texture
    uv -= center;

    // Scale the UV coordinates
    uv *= scale;

    // Rotate the UV coordinates
    uv = rotate2D(uv, rotate);

    // Translate the UV coordinates back to their original position
    uv += center;
  }

  vec4 mainColor = texture(mainTexture, coord);
  
  // If the resultant position is outside the frame, fbColor should be empty
  vec2 fbCoord = uv - translate;
  vec4 fbColor = vec4(0.0);
  if (!(fbCoord.x > 1.0 || fbCoord.y > 1.0 || fbCoord.x < 0.0 ||
        fbCoord.y < 0.0)) {
    fbColor = texture(fbTexture, uv - translate);
  }
  vec4 outColor = vec4(0.0, 0.0, 0.0, 0.0);

  // If both are empty
  if (mainColor.a < 0.0001 && fbColor.a < 0.0001) {
    outputColor = outColor;
    return;
  }
  
  // If we have priority turned on and we're in a main pixel
  if (mainColor.a > 0.001 && priority > 0) {
    outputColor = mainColor;
    return;
  }

  if (lumaEnabled == 1) {
    fbColor = mixLumaKey(mainColor, fbColor, lumaKey, lumaThresh);
  }
  
  outColor = blend(mainColor, fbColor, blendMode);

  outputColor = outColor;
}
