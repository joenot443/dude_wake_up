#version 150

uniform sampler2D tex;
uniform vec2 dimensions;
uniform float time;
uniform float weight;
uniform vec4 strokeColor;
in vec2 coord;
out vec4 outputColor;


void main() {
  
  const float targetAlpha = 0.0;
  const float TAU = 6.28318530;
  const float steps = 32.0;
  
  float radius = weight;
  vec2 uv = coord / dimensions.xy;
  
  // Correct aspect ratio
  vec2 aspect = 1.0 / vec2(textureSize(tex, 0));
  
  vec4 texColor = texture(tex, uv);
  for (float i = 0.0; i < TAU; i += TAU / steps) {
    // Sample image in a circular pattern
    vec2 offset = vec2(sin(i), cos(i)) * aspect * radius;
    vec4 col = texture(tex, uv + offset);
    
    // Mix outline with background
    float alpha = smoothstep(0.5, 0.7, distance(col.a, targetAlpha));
    texColor = mix(texColor, vec4(strokeColor.rgb, 1.0), alpha);
  }
  
  // Overlay original video
  vec4 mat = texture(tex, uv);
  float factor = smoothstep(0.5, 0.7, distance(mat.a, targetAlpha));
  outputColor = mix(texColor, mat, factor);
}
