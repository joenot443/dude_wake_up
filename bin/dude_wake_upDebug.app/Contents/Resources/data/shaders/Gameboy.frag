#version 150
#define mode 1  //0: Game Boy, 1: Game Boy Color

uniform sampler2D tex;
uniform sampler2D tex2;
uniform vec2 dimensions;
uniform float time;
in vec2 coord;
out vec4 outputColor;

void main(){
  vec2 res = vec2(128,112); //Gameboy Camera's screen resolution
  vec2 uvRes = vec2(floor(res.y*(dimensions.x/dimensions.y)),res.y); //Widescreen version of the Gameboy's screen resolution
  
  
  vec2 uv = coord/dimensions.xy;
  vec2 pos = floor(uv*uvRes);
  
  vec3 col = vec3(0);
  
  if (uvRes.x/2. - res.x/2. < pos.x && uvRes.x/2. + res.x/2. > pos.x){
    if(mode == 0){
      //use specific weighted amounts of each channel to improve the grayscale colors (Luma values)
      float gray = dot(texture(tex,pos/uvRes).rgb, vec3(0.2126,0.7152,0.0722));
      float dither = texture(tex2, pos/8.).r;
      
      col = vec3(floor(gray*3.+dither)/3.);
    }else{
      vec3 tex = texture(tex,pos/uvRes).rgb;
      float dither = texture(tex2, pos/8.).r;
      //downscale the colors to be within the GBC's limitations (around 56 colors max according to the Wikipedia article)
      vec3 newCol = (floor(tex*4. + dither))/4.;
      
      col = newCol;
    }
  }
  
  outputColor = vec4(col,1.0);
}
