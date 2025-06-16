#version 150

uniform sampler2D tex;
in vec2 coord;
out vec4 outputColor;
uniform float amount;

uniform vec2 dimensions;
uniform vec3 hsbScalar;

#define M_PI 3.14159265

vec3 TransformHSV(
                  vec3 inputCol,  // color to transform
                  float h,          // hue shift (in degrees)
                  float s,          // saturation multiplier (scalar)
                  float v           // value multiplier (scalar)
                  ) {
                    float vsu = v * s * cos(h * M_PI/180.0);
                    float vsw = v * s * sin(h * M_PI/180.0);
                    
                    vec3 ret = vec3(0.0);
                    ret.r = (.299*v + .701*vsu + .168*vsw)*inputCol.r
                    +   (.587*v - .587*vsu + .330*vsw)*inputCol.g
                    +   (.114*v - .114*vsu - .497*vsw)*inputCol.b;
                    ret.g = (.299*v - .299*vsu - .328*vsw)*inputCol.r
                    +   (.587*v + .413*vsu + .035*vsw)*inputCol.g
                    +   (.114*v - .114*vsu + .292*vsw)*inputCol.b;
                    ret.b = (.299*v - .300*vsu + 1.25*vsw)*inputCol.r
                    +   (.587*v - .588*vsu - 1.05*vsw)*inputCol.g
                    +   (.114*v + .886*vsu - .203*vsw)*inputCol.b;
                    
                    return ret;
                  }

void main()
{
  
  // Normalized pixel coordinates (from 0 to 1)
  vec4 col = texture(tex, coord);
  
  vec3 newCol = clamp(TransformHSV(col.xyz, hsbScalar.x * 360.0, hsbScalar.y, hsbScalar.z), 0.0, 1.0);
  vec3 finalCol = mix(col.xyz, newCol, amount);
  outputColor = vec4(finalCol, col.a);
}


