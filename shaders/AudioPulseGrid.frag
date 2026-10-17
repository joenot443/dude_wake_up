#version 150

uniform sampler2D tex;
uniform vec2 dimensions;
uniform float time;

uniform int columns;
uniform float dotSize;
uniform float pulseStrength;
uniform float glowFalloff;
uniform float colorShift;
uniform float audioSensitivity;
uniform float brightness;
uniform float decay;

uniform float audio[256];

in vec2 coord;
out vec4 outputColor;

#define PI 3.14159265359

vec3 hsv2rgb(vec3 c) {
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

void main() {
    // Centered, aspect-corrected coordinates (like AudioBall)
    vec2 p = (coord.xy - 0.5 * dimensions.xy) / min(dimensions.x, dimensions.y);

    // Start from black background
    vec3 c = vec3(0.0);

    // Total audio energy for silent fallback
    float totalEnergy = 0.0;
    for (int i = 0; i < 256; i += 8) {
        totalEnergy += audio[i];
    }

    int cols = clamp(columns, 4, 32);
    float aspect = dimensions.x / dimensions.y;
    int rows = max(int(float(cols) / aspect), 2);

    // Grid spacing in the normalized coordinate space
    float spacingX = aspect / float(cols);
    float spacingY = 1.0 / float(rows);

    // Grid origin (centered)
    float gridW = spacingX * float(cols);
    float gridH = spacingY * float(rows);
    float originX = -gridW * 0.5;
    float originY = -gridH * 0.5;

    // Glow size factor
    float glowSize = dotSize * spacingX * 0.5;
    float eps = glowSize * 0.15 / max(glowFalloff * 0.3, 0.01);

    // Iterate over ALL dots (like AudioBall iterates over all dots)
    // For grids up to 32x18 = 576 dots, this is fine
    // But for performance, only visit nearby dots
    // Find which cell we're nearest to, then iterate neighborhood
    int nearX = int((p.x - originX) / spacingX);
    int nearY = int((p.y - originY) / spacingY);

    for (int dy = -3; dy <= 3; dy++) {
        for (int dx = -3; dx <= 3; dx++) {
            int cx = nearX + dx;
            int cy = nearY + dy;

            if (cx < 0 || cx >= cols || cy < 0 || cy >= rows) continue;

            // Dot center position
            float dotX = originX + (float(cx) + 0.5) * spacingX;
            float dotY = originY + (float(cy) + 0.5) * spacingY;
            vec2 dotPos = vec2(dotX, dotY);

            // Frequency amplitude for this column
            // Cyclic scrolling: band position rotates over time
            float scrollOffset = time * decay * 0.3;
            float bandCenter = fract(float(cx) / float(cols) + scrollOffset);

            // Feathered sampling: 6 points across a wider band to smooth transitions
            float bandWidth = 1.0 / float(cols);
            float amp = 0.0;
            for (int s = 0; s < 6; s++) {
                float offset = (float(s) - 2.5) / 5.0 * bandWidth * 2.0;
                float t = fract(bandCenter + offset); // wraps cyclically
                int idx = clamp(int(t * 255.0), 0, 255);
                amp += audio[idx];
            }
            amp *= (1.0 / 6.0) * audioSensitivity;

            // Silent fallback
            if (totalEnergy < 0.05) {
                float phase = float(cx * 3 + cy * 7) * 0.5 + time * 0.7;
                amp = 0.15 + 0.1 * sin(phase);
            }

            // Row attenuation: bottom brighter, top dimmer
            float row_t = float(cy) / float(rows);
            amp *= 1.0 - row_t * 0.3;

            // Audio-driven glow brightness (like AudioBall: b/length(p-o))
            float vol = amp * pulseStrength;
            float b = vol * dotSize * 0.02;

            float dist = length(p - dotPos);

            // Color: hue follows the frequency band (scrolls with content)
            float hue = colorShift + bandCenter * 0.7;
            float sat = mix(0.9, 0.2, smoothstep(0.4, 1.5, amp));
            vec3 dotCol = hsv2rgb(vec3(hue, sat, 1.0));

            // Additive glow accumulation (AudioBall technique)
            c += b / (dist + eps) * dotCol * brightness;
        }
    }

    // Reinhard tone mapping — prevents blowout to white, preserves color
    c = c / (1.0 + c);

    outputColor = vec4(c, 1.0);
}
