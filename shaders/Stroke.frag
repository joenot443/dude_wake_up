#version 150

uniform sampler2D tex;
uniform vec2 dimensions;
uniform float time;
uniform float amount;
uniform vec4 strokeColor;
in vec2 coord;
out vec4 outputColor;


void main() {
  
  const vec3 target = vec3(0.0, 0.0, 0.0);
  const float TAU = 6.28318530;
  const float steps = 32.0;
  
  float radius = 10.0;
  vec2 uv = coord / dimensions.xy;
  
  // Correct aspect ratio
  vec2 aspect = 1.0 / vec2(textureSize(tex, 0));
  
  vec4 texColor = texture(tex, uv);
  for (float i = 0.0; i < TAU; i += TAU / steps) {
    // Sample image in a circular pattern
    vec2 offset = vec2(sin(i), cos(i)) * aspect * radius;
    vec4 col = texture(tex, uv + offset);
    
    // Mix outline with background
    float alpha = smoothstep(0.5, 0.7, distance(col.rgb, target));
    texColor = mix(texColor, vec4(strokeColor.rgb, 1.0), alpha);
  }
  
  // Overlay original video
  vec4 mat = texture(tex, uv);
  float factor = smoothstep(0.5, 0.7, distance(mat.rgb, target));
  outputColor = mix(texColor, mat, factor);
}
