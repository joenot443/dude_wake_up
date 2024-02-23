#version 150

uniform sampler2D tex;
uniform vec2 dimensions;
uniform float time;
uniform float xShift;
uniform float yShift;
uniform float scale;
uniform int drawLeft;
uniform int drawCenter;
uniform int drawRight;
in vec2 coord;
out vec4 outputColor;

void main()
{
    // Convert screen dimensions to UV space
    vec2 uvDimensions = vec2(1.0, dimensions.y / dimensions.x);

    // Center for scaling
    vec2 center = vec2(0.5, 0.5);

    // Calculate scaled and shifted coordinates for three textures
    vec2 scaledCoord = (coord - center) * scale + center; // Scale around the center
    vec2 coords[3];
    coords[0] = scaledCoord + vec2(-uvDimensions.x / 3.0 + xShift, yShift); // Left
    coords[1] = scaledCoord + vec2(xShift, yShift);                         // Center
    coords[2] = scaledCoord + vec2(uvDimensions.x / 3.0 + xShift, yShift);  // Right

    // Initialize color sum and count for averaging
    vec4 colorSum = vec4(0.0);
    float count = 0.0;

    // Sum colors and count only for non-transparent pixels and if drawing is enabled
    int drawFlags[3];
    drawFlags[0] = drawLeft;
    drawFlags[1] = drawCenter;
    drawFlags[2] = drawRight;

    for (int i = 0; i < 3; i++) {
        if (drawFlags[i] == 1) {
            vec4 texSample = texture(tex, coords[i]);
            if (texSample.a > 0.0) {
                colorSum += texSample;
                count += 1.0;
            }
        }
    }

    // Handle the case when count is zero
    if (count == 0.0) {
        outputColor = vec4(0.0); 
    } else {
        colorSum /= count;
        outputColor = vec4(colorSum.rgb, 1.0);
    }
}
