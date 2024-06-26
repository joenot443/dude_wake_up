#version 150

uniform sampler2D tex;
uniform vec2 dimensions;
uniform float spacing;
uniform float shape;
in vec2 coord;
uniform float time;
out vec4 outputColor;

// 2D rotation function
mat2 rot2D(float a) {
  return mat2(cos(a), -sin(a), sin(a), cos(a));
}

// Custom gradient - https://iquilezles.org/articles/palettes/
vec3 palette(float t) {
  return .5+.5*cos(6.28318*(t+vec3(.3,.416,.557)));
}

// Octahedron SDF - https://iquilezles.org/articles/distfunctions/
float sdOctahedron(vec3 p, float s) {
  p = abs(p);
  return (p.x+p.y+p.z-s)*0.57735027;
}

// Scene distance
float map(vec3 p) {
  p.z += time * .4; // Forward movement
  
  // Space repetition
  p.xy = fract(p.xy)*shape - .5;     // spacing: 1
  p.z =  mod(p.z, spacing) - .125; // spacing: .25
  
  return sdOctahedron(p, .15); // Octahedron
}

void main(  ) {
  vec2 uv = vec2(coord.x/dimensions.x, coord.y/dimensions.y);
  vec2 m = vec2(cos(time*.2), sin(time*.2));
  
  // Initialization
  vec3 ro = vec3(0, 0, -3);         // ray origin
  vec3 rd = normalize(vec3(uv, 1)); // ray direction
  vec3 col = vec3(0);               // final pixel color
  
  float t = 0.; // total distance travelled
  
  int i = 0; // Raymarching
  for (i = 0; i < 80; i++) {
    vec3 p = ro + rd * t; // position along the ray
    
    p.xy *= rot2D(t*.15 * m.x);     // rotate ray around z-axis
    
    p.y += sin(t*(m.y+1.))*.35;  // wiggle ray
    
    float d = map(p);     // current distance to the scene
    
    t += d;               // "march" the ray
    
    if (d < .001 || t > 100.) {
      break;
    }; // early stop
  }
  
  // Coloring
  col = palette(t*.04 + 80*.005);
  
  outputColor = vec4(col, 1);
}
