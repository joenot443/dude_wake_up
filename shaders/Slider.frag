#version 150

uniform float time;    // time uniform
uniform float speed;   // speed uniform
uniform sampler2D tex; // texture sampler

in vec2 coord; // texture coordinates

out vec4 outputColor; // fragment color

void main() {
  // Calculate the offset for the texture based on the current time and speed
  float offset = mod(time * speed, 1.0);

  // Shift the texture coordinates based on the offset
  vec2 coordShifted = coord - vec2(offset, 0);

  // Repeat the texture horizontally by using the modulo operator
  coordShifted.x = mod(coordShifted.x, 1.0);

  // Sample the texture at the shifted texture coordinates
  outputColor = texture(tex, coordShifted);
}
