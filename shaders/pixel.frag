#version 150

uniform sampler2D tex;
in vec2 coord;
out vec4 outputColor;
uniform vec2 dimensions;
uniform float size;
uniform float gradient;

void main()
{
  vec2 fragCoord = coord * dimensions;

  // Power < 1 compresses left side in warped space, creating small pixels
  // on the left and large pixels on the right when mapped back
  float power = mix(1.0, 0.3, gradient);
  float nx = fragCoord.x / dimensions.x;

  // Warp x into u-space
  float ux = pow(nx, power);

  // Pixelate uniformly in u-space
  float cellW = size / dimensions.x;
  float snappedUx = floor(ux / cellW) * cellW;

  // Map column edges back to screen space
  float colLeft = pow(snappedUx, 1.0 / power) * dimensions.x;
  float colRight = pow(min(snappedUx + cellW, 1.0), 1.0 / power) * dimensions.x;
  float localSize = max(colRight - colLeft, 1.0);

  // Use the column's screen-space width for y snapping too, giving square blocks
  float snappedY = floor(fragCoord.y / localSize) * localSize;

  outputColor = texture(tex, vec2(colLeft, snappedY) / dimensions);
}
