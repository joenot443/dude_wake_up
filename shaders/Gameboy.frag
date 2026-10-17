#version 150

uniform sampler2D tex;
uniform sampler2D tex2;
uniform vec2 dimensions;
uniform float scale;
uniform float time;
uniform int mode;
uniform vec4 tintColor;
uniform int levels;
uniform float ditherAmount;
uniform float pixelGrid;
in vec2 coord;
out vec4 outputColor;

void main() {
  vec2 res = vec2(128 * scale, 112 * scale); //Gameboy Camera's screen resolution
  vec2 uvRes = vec2(floor(res.y*(dimensions.x/dimensions.y)), res.y); //Widescreen version of the Gameboy's screen resolution


  vec2 uv = coord/dimensions.xy;
  vec2 pos = floor(uv*uvRes);

  vec3 col = vec3(0);
  float L = float(levels);

  if(mode == 0){
    //use specific weighted amounts of each channel to improve the grayscale colors (Luma values)
    float gray = dot(texture(tex ,pos/uvRes).rgb, vec3(0.2126,0.7152,0.0722));
    float dither = (texture(tex2, pos/8.).r - 0.5) * ditherAmount + 0.5;

    col = vec3(floor(gray*(L-1.)+dither)/(L-1.));
  } else {
    vec3 tex = texture(tex,pos/uvRes).rgb;
    float dither = (texture(tex2, pos/8.).r - 0.5) * ditherAmount + 0.5;
    //downscale the colors to be within the GBC's limitations (around 56 colors max according to the Wikipedia article)
    vec3 newCol = (floor(tex*L + dither))/L;

    col = newCol;
  }

  // Tint (multiplicative) — default white = no change
  col *= tintColor.rgb;

  // LCD pixel-grid darkening between pixels
  vec2 fr = fract(uv * uvRes);
  vec2 edgeDist = min(fr, 1.0 - fr);
  float gridFactor = smoothstep(0.0, 0.15, min(edgeDist.x, edgeDist.y));
  col *= mix(1.0, gridFactor, pixelGrid);

  outputColor = vec4(col,1.0);
}
