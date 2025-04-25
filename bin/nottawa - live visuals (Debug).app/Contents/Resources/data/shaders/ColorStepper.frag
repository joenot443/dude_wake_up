#version 150

uniform sampler2D tex;
uniform vec2 dimensions;
uniform float time;
uniform float amount;
uniform float speed;
uniform float smoothness;
in vec2 coord;
out vec4 outputColor;


// MODIFY THIS: color stepping size
vec4 stepSize = vec4(smoothness * 1000.0);

void main(  )
{
  // Normalized pixel coordinates (from 0 to 1)
  vec2 uv = coord/dimensions.xy;
  vec4 col = 0.5 + 0.5*cos(time/speed + uv.xyxy+vec4(0,2,4,1.0));
  vec4 text = texture(tex, uv);
  
  col *= text;
  
  // step the color range
  col *= stepSize;
  
  col = round(col);
  
  col /= stepSize;
  
  // Output to screen
  outputColor = mix(text, vec4(col.xyz, text.a), amount);
}
