#version 150

uniform sampler2D tex;
uniform vec2 dimensions;
uniform float time;
in vec2 coord;
out vec4 outputColor;
void main() {

  const vec3 target = vec3(0.0, 1.0, 0.0); // Find green
  const float TAU = 6.28318530;
  const float steps = 32.0;
    
  float radius = 20.0;
  vec2 uv = coord / dimensions.xy;
    
  // Correct aspect ratio
  vec2 aspect = 1.0 / vec2(textureSize(tex, 0));
    
  outputColor = vec4(uv.y, 0.0, uv.x, 1.0);
  for (float i = 0.0; i < TAU; i += TAU / steps) {
    // Sample image in a circular pattern
    vec2 offset = vec2(sin(i), cos(i)) * aspect * radius;
    vec4 col = texture(tex, uv + offset);    
  }
  
    // Overlay original video
  vec4 mat = texture(tex, uv);
  float factor = smoothstep(0.5, 0.7, distance(mat.rgb, target));
  outputColor = mix(outputColor, mat, factor);
}
