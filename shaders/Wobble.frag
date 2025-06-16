#version 150

// Input texture and parameters
uniform sampler2D tex;
uniform float time;
uniform float speed;
uniform float amount;
in vec2 coord;
out vec4 outputColor;

void main()
{
  // Scale and center UV coordinates (0.0-1.0 range with padding)
  vec2 uv = coord * 0.8 + 0.1;
  
  // Apply wobble effect - cosine displacement based on time and position
  vec2 wobbleOffset = cos(time * speed * vec2(6.0, 7.0) + uv * 10.0) * 0.02 * amount;
  uv += wobbleOffset;
  
  // Sample the texture with the wobbled coordinates
  outputColor = texture(tex, uv);
}


