#version 150

uniform sampler2D tex;
uniform sampler2D tex2;
uniform vec2 dimensions;
uniform float time;
in vec2 coord;
out vec4 outputColor;

uniform float lineWidth;
uniform float speed;
uniform float angle; // Angle for the line
uniform float position; // Static position when speed is 0

void main()
{
  vec2 uv = coord.xy / dimensions.xy;
  
  // Calculate the moving line's position based on speed
  float linePosition = speed == 0.0 ? position : mod(time * speed, dimensions.x);
  
  // Calculate pivot for rotation based on current line position
  vec2 pivot = vec2(0.0, dimensions.y * 0.5); // Pivot in the middle of the height
  
  // Translate coordinates to pivot for rotation
  vec2 translatedCoord = coord.xy - vec2(linePosition, pivot.y);
  
  // Rotate coordinates around pivot
  float cosAngle = cos(angle);
  float sinAngle = sin(angle);
  vec2 rotatedCoord;
  rotatedCoord.x = cosAngle * translatedCoord.x - sinAngle * translatedCoord.y;
  rotatedCoord.y = sinAngle * translatedCoord.x + cosAngle * translatedCoord.y;
  
  // Determine if the pixel is within the line width after rotation
  bool withinLineWidth = abs(rotatedCoord.x) <= lineWidth * 0.5;
  
  // Logic for drawing the line or sampling textures
  if (withinLineWidth)
  {
    outputColor = vec4(0.0, 0.0, 0.0, 1.0); // Draw black line
  }
  else
  {
    // Use rotated coordinates to determine which texture to sample
    if (rotatedCoord.x <= 0.0)
    {
      outputColor = texture(tex, uv); // Sample from tex
    }
    else
    {
      outputColor = texture(tex2, uv); // Sample from tex2
    }
  }
}
