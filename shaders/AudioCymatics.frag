#version 150

uniform sampler2D tex;
uniform vec2 dimensions;
uniform float time;

// Parameters
uniform int modeCount;
uniform float sharpness;
uniform float scale;
uniform float rotation;
uniform float colorShift;
uniform float audioSensitivity;
uniform float brightness;
uniform float decay;

// Audio spectrum (256 frequency bins, 0-1 range)
uniform float audio[256];

in vec2 coord;
out vec4 outputColor;

#define PI 3.14159265359

// Chladni mode pairs ordered by increasing complexity
// Low indices = bass (simple patterns), high indices = treble (complex patterns)
const vec2 modes[12] = vec2[12](
    vec2(1.0, 2.0), vec2(2.0, 3.0), vec2(1.0, 3.0), vec2(3.0, 4.0),
    vec2(2.0, 4.0), vec2(1.0, 4.0), vec2(3.0, 5.0), vec2(4.0, 5.0),
    vec2(2.0, 5.0), vec2(1.0, 5.0), vec2(5.0, 6.0), vec2(3.0, 6.0)
);

vec3 hsv2rgb(vec3 c) {
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

// Average 4 bins across the frequency band for this mode index
// This prevents single-bin flickering
float getModeAmplitude(int modeIndex, int totalModes) {
    float bandStart = float(modeIndex) / float(totalModes);
    float bandEnd = float(modeIndex + 1) / float(totalModes);
    float sum = 0.0;
    for (int j = 0; j < 4; j++) {
        float t = mix(bandStart, bandEnd, float(j) / 3.0);
        int idx = clamp(int(t * 255.0), 0, 255);
        sum += audio[idx];
    }
    return sum * 0.25;
}

void main() {
    // Normalize to centered coordinates with uniform aspect ratio
    vec2 uv = (coord.xy - 0.5 * dimensions.xy) / min(dimensions.x, dimensions.y);

    // Apply scale
    uv *= scale;

    // Apply rotation
    float angle = rotation * time;
    mat2 rot = mat2(cos(angle), -sin(angle), sin(angle), cos(angle));
    uv = rot * uv;

    // Shift to [0, 1] domain for the Chladni formula
    // (plate extends from 0 to 1)
    vec2 plate = uv + 0.5;

    // Detect total audio energy for silent fallback
    float totalEnergy = 0.0;
    for (int i = 0; i < 256; i += 8) {
        totalEnergy += audio[i];
    }

    // Compute displacement field as weighted sum of Chladni modes
    float displacement = 0.0;
    float totalAmp = 0.0;
    float dominantHue = 0.0;
    float dominantWeight = 0.0;

    int mc = clamp(modeCount, 2, 12);

    for (int i = 0; i < mc; i++) {
        float n = modes[i].x;
        float m = modes[i].y;

        // Get audio amplitude for this mode's frequency band
        float amp;
        if (totalEnergy < 0.05) {
            // Silent fallback: gentle time-varying pattern
            amp = 0.15 + 0.1 * sin(time * 0.5 + float(i) * 1.7);
        } else {
            amp = getModeAmplitude(i, mc) * audioSensitivity;
        }

        totalAmp += amp;

        // Chladni mode: sin(n*pi*x)*sin(m*pi*y) + sin(m*pi*x)*sin(n*pi*y)
        float mode = sin(n * PI * plate.x) * sin(m * PI * plate.y)
                   + sin(m * PI * plate.x) * sin(n * PI * plate.y);

        displacement += amp * mode;

        // Track which mode is contributing most (for color)
        float contribution = amp * abs(mode);
        if (contribution > dominantWeight) {
            dominantWeight = contribution;
            dominantHue = float(i) / float(mc);
        }
    }

    // Scale output by total vibration energy — no energy = dark, not white
    float energyScale = clamp(totalAmp * 2.0, 0.0, 1.0);

    // Gaussian falloff at nodal lines: bright where displacement ~ 0
    float pattern = exp(-sharpness * displacement * displacement);

    // Color mapping based on displacement magnitude
    float normalizedDisp = clamp(abs(displacement) * 0.5, 0.0, 1.0);

    // Base hue from colorShift, shifted by displacement magnitude
    float hue = colorShift + normalizedDisp * 0.3 + dominantHue * 0.15;
    // Saturation: nodal lines are slightly less saturated (more white/bright)
    float sat = 0.5 + normalizedDisp * 0.4;
    // Value: driven by the Gaussian pattern, scaled by energy
    float val = pattern * brightness * energyScale;

    vec3 color = hsv2rgb(vec3(hue, sat, val));

    // Add subtle secondary glow for depth
    float softGlow = exp(-sharpness * 0.1 * displacement * displacement);
    vec3 glowColor = hsv2rgb(vec3(colorShift + 0.5, 0.3, softGlow * brightness * 0.15));
    color += glowColor;

    // Decay: blend with previous frame for temporal trailing
    vec2 texUv = coord.xy / dimensions.xy;
    vec4 prevColor = texture(tex, texUv);
    color = mix(color, prevColor.rgb, decay);

    outputColor = vec4(color, 1.0);
}
