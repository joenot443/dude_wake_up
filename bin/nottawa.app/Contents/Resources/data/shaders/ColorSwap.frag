#version 150

// Add uniforms to select channels
uniform sampler2D tex;
uniform sampler2D tex2;
uniform float blend;
uniform int mainRGB; // 0 for R, 1 for G, 2 for B
uniform int auxRGB; // 0 for R, 1 for G, 2 for B
uniform vec2 dimensions;

in vec2 coord;
out vec4 outputColor;

void main()
{
  // Fetch the color from both textures
  vec4 color0 = texture(tex, coord);
  vec4 color1 = texture(tex2, coord);
  
  // Use the selected component from channel1
  float componentFromChannel1 = color1[auxRGB];
  
  // Set the resulting color starting with color0
  outputColor = color0; // Use color0 as the base
  
  // Overwrite the selected component in outputColor with the chosen component from channel1
  outputColor[mainRGB] = mix(outputColor[mainRGB], componentFromChannel1, blend);
}
