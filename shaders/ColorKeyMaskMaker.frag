#version 150

uniform float tolerance;
uniform sampler2D tex;
uniform vec2 dimensions;
uniform float time;
uniform vec4 chromaKey;


in vec2 coord;
out vec4 outputColor;

mat4 RGBtoYUV = mat4(0.257,  0.439, -0.148, 0.0,
                     0.504, -0.368, -0.291, 0.0,
                     0.098, -0.071,  0.439, 0.0,
                     0.0625, 0.500,  0.500, 1.0 );



//compute color distance in the UV (CbCr, PbPr) plane
float colorclose(vec3 yuv, vec3 keyYuv)
{
  float tmp = sqrt(pow(keyYuv.g - yuv.g, 2.0) + pow(keyYuv.b - yuv.b, 2.0));
  if (tmp < tolerance)
    return 1.0;
  else
    return 0.0;
}


void main()
{
  vec2 fragPos =  coord.xy / dimensions.xy;
  vec4 texColor0 = texture(tex, fragPos);
  
  //convert from RGB to YCvCr/YUV
  vec4 keyYUV =  RGBtoYUV * chromaKey;
  vec4 yuv = RGBtoYUV * texColor0;
  
  float mask = colorclose(yuv.rgb, keyYUV.rgb);
  outputColor = vec4(1.0, 1.0, 1.0, mask);
}

