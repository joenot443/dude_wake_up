#version 150
#define mode 1  //0: Game Boy, 1: Game Boy Color

uniform sampler2D tex;
uniform sampler2D tex2;
uniform vec2 dimensions;
uniform float time;
in vec2 coord;
out vec4 outputColor;

void main(){
  vec2 uv = coord/dimensions.xy; // Use the entire texture space
  vec4 text = texture(tex, uv); // Fetch the texture color
  
  vec3 col = vec3(0);
  
  if(mode == 0){
    // Apply grayscale color transformation for the entire texture
    float gray = dot(text.rgb, vec3(0.2126, 0.7152, 0.0722));
    float dither = texture(tex2, uv*dimensions/8.).r; // Adjust texture coordinates for dithering
    col = vec3(floor(gray*3.+dither)/3.);
  } else {
    // Apply Game Boy Color styling for the entire texture
    float dither = texture(tex2, uv*dimensions/8.).r; // Adjust texture coordinates for dithering
    vec3 newCol = (floor(text.rgb*4. + dither))/4.; // Downscale colors
    col = newCol;
  }
  
  outputColor = vec4(col, text.a); // Set output color
}
