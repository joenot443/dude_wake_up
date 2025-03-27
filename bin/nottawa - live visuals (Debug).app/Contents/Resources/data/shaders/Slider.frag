#version 150

uniform float time;    // time uniform
uniform float speed;   // speed uniform
uniform sampler2D tex; // texture sampler
uniform int vertical;

in vec2 coord; // texture coordinates

out vec4 outputColor; // fragment color

void main() {
  // Calculate the offset for the texture based on the current time and speed
  float offset = mod(time * speed, 1.0);
  vec2 coordShifted = vec2(0,0);
  if (vertical > 0) {
    // Repeat the texture horizontally by using the modulo operator
    coordShifted = coord - vec2(0, offset);
    coordShifted.y = mod(coordShifted.y, 1.0);
  } else {
    // Repeat the texture horizontally by using the modulo operator
    coordShifted = coord - vec2(offset, 0);
    coordShifted.x = mod(coordShifted.x, 1.0);
  }
  
  // Sample the texture at the shifted texture coordinates
  outputColor = texture(tex, coordShifted);
}
