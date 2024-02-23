#version 150

uniform sampler2D mainTexture;
uniform sampler2D fbTexture;
uniform sampler2D overlayTexture;

in vec2 coord;
out vec4 outputColor;

uniform float mainMix;
uniform float feedbackMix;
uniform int fbType;

uniform float scale;
uniform vec2 translate;
uniform int lumaEnabled;
uniform int overlayEnabled;
uniform float lumaKey;
uniform float lumaThresh;
uniform vec3 hsb;
uniform vec3 rescale;
uniform float rotate;
uniform vec3 invert;


vec3 rgb2hsb( in vec3 c ){
  vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
  vec4 p = mix(vec4(c.bg, K.wz),
               vec4(c.gb, K.xy),
               step(c.b, c.g));
  vec4 q = mix(vec4(p.xyw, c.r),
               vec4(c.r, p.yzx),
               step(p.x, c.r));
  float d = q.x - min(q.w, q.y);
  float e = 1.0e-10;
  return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)),
              d / (q.x + e),
              q.x);
}

vec3 hsb2rgb( in vec3 c ){
  vec3 rgb = clamp(abs(mod(c.x*6.0+vec3(0.0,4.0,2.0),
                           6.0)-3.0)-1.0,
                   0.0,
                   1.0 );
  rgb = rgb*rgb*(3.0-2.0*rgb);
  return c.z * mix(vec3(1.0), rgb, c.y);
}

vec4 mixStandard(in vec4 mainColor,
                 in vec4 fbColor) {
  vec4 overlayColor = fbColor;
  
//  if (overlayEnabled == 1) {
//    overlayColor = texture(overlayTexture, coord);
//  }
  
  // Empty feedback
  if (fbColor.a < 0.5) {
    return mainColor;
  }
  
  // Empty main
  if (mainColor.a < 0.0001) {
    return vec4(overlayColor.rgb, min(0.99, 1.5 * feedbackMix * overlayColor.a));
  }
  
  return mix(mainColor, overlayColor, mainMix);
}
      

vec4 mixLumaKey(in vec4 mainColor,
                in vec4 fbColor,
                in float key,
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
    return mix(mainColor, fbColor, feedbackMix);
  }
  
  return mainColor;
}

void main()
{
  vec2 uv = coord;
  // Scale if necessary
  if (abs(1.0 - scale) > 0.01) {
    vec2 center = vec2(0.5, 0.5); // Center of the texture
    
    // Translate the UV coordinates to center the texture
    uv -= center;
    
    // Scale the UV coordinates
    uv *= scale;
    
    // Translate the UV coordinates back to their original position
    uv += center;
  }
  
  vec4 mainColor = texture(mainTexture, coord);
  // If the resultant position is outside the frame, fbColor should be empty
  vec2 fbCoord = uv - translate;
  vec4 fbColor = vec4(0.0);
  if (!(fbCoord.x > 1.0 || fbCoord.y > 1.0 || fbCoord.x < 0.0 || fbCoord.y < 0.0)) {
    fbColor = texture(fbTexture, uv - translate);
  }
  vec4 outColor = vec4(0.0, 0.0, 0.0, 0.0);
  
  // If both are empty
  if (mainColor.a < 0.0001 && fbColor.a < 0.0001) {
    outputColor = outColor;
    return;
  }
  
  if (lumaEnabled == 1) {
    outColor = mixLumaKey(mainColor, fbColor, lumaKey, lumaThresh);
  } else {
    outColor = mixStandard(mainColor, fbColor);
  }
  
  outputColor = outColor;
}

