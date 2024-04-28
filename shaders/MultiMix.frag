#version 150

uniform sampler2D tex[10];  // Array of textures
uniform int tex_active[10]; // Array indicating active status of each texture
uniform float mix[10];      // Array of mix factors for each texture

in vec2 coord;
out vec4 outputColor;

void main()
{
    int activeCount = 0;
    vec4 accumulatedColor = vec4(0.0);
    float accumulatedAlpha = 0.0;
    float totalMixFactor = 0.0;

    // Sum active textures and calculate total mix
    for (int i = 0; i < 10; i++) {
        if (tex_active[i] != 0) {
            vec4 tex_color = texture(tex[i], coord);
            if (tex_color.a > 0.1) {
                accumulatedColor += tex_color * mix[i];
                accumulatedAlpha += tex_color.a * mix[i];
                totalMixFactor += mix[i];
                activeCount++;
            }
        }
    }

    if (activeCount == 0 || totalMixFactor == 0.0) {
        outputColor = vec4(0.0);
    } else {
        outputColor = vec4(accumulatedColor.rgb / totalMixFactor, accumulatedAlpha / totalMixFactor);
    }
}
