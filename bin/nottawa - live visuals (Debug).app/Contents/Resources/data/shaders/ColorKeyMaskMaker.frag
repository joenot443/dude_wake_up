#version 150

uniform float tolerance;
uniform sampler2D tex;
uniform sampler2D auxTex;
uniform vec2 dimensions;
uniform float time;
uniform vec4 chromaKey;
uniform int drawTex;
uniform int drawBackground;
uniform int invert;

in vec2 coord;
out vec4 outputColor;

mat4 RGBtoYUV = mat4(0.257,  0.439, -0.148, 0.0,
                     0.504, -0.368, -0.291, 0.0,
                     0.098, -0.071,  0.439, 0.0,
                     0.0625, 0.500,  0.500, 1.0 );



//compute color distance in the UV (CbCr, PbPr) plane
float colorclose(vec3 yuv, vec3 keyYuv)
{
  return sqrt(pow(keyYuv.g - yuv.g, 2.0) + pow(keyYuv.b - yuv.b, 2.0));
}


void main()
{
  vec2 fragPos = coord.xy / dimensions.xy;
  vec4 texColor0 = texture(tex, fragPos);
  vec4 auxColor = texture(auxTex, fragPos);
  
  // Convert from RGB to YCbCr/YUV
  vec4 keyYUV = RGBtoYUV * chromaKey;
  vec4 yuv = RGBtoYUV * texColor0;
  
  float mask = colorclose(yuv.rgb, keyYUV.rgb);
  
  if (mask >= tolerance) {
    // If the pixel matches the mask color
    if (drawTex == 1) {
      outputColor = auxColor; // Use auxTex for matching pixels
    } else {
      outputColor = vec4(0.0); // Use vec4(0.0) if drawTex is false
    }
  } else {
    // If the pixel does not match the mask color
    if (drawBackground == 1) {
      outputColor = texColor0; // Use tex for non-matching pixels
    } else {
      outputColor = vec4(0.0); // Use vec4(0.0) if drawBackground is false
    }
  }
}

