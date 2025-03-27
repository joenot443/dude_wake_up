#version 150

uniform sampler2D tex;
uniform vec2 dimensions;
uniform float time;
uniform float brightness;
in vec2 coord;
out vec4 outputColor;


void main()
{
  // Normalized pixel coordinates (from 0 to 1)
  vec2 uv = coord / dimensions.xy;
  
  // Time-varying pixel color calculation
  vec3 col = 0.5 + 0.5 * cos(time + uv.xyx + vec3(0, 2, 4));
  
  // Sample texture at UV coordinates
  vec4 temp = texture(tex, uv);
  
  // If the alpha is almost zero, discard the fragment (output transparent)
  if (temp.a < 0.01) {
    outputColor = vec4(0.0);
    return;
  }
  
  // Modify color based on texture and apply a tan function for color warping
  col = temp.rgb * brightness
  + vec3(tan((temp.rgb * vec3(3.0, 5.0, 50.0)
              + (time / 135.0) * 100.0) * 3.1415)) * vec3(1.0, 0.1, 1.0);
  
  // Output the final color with alpha set to 1
  outputColor = vec4(col, 1.0);
}
