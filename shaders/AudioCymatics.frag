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
    vec2 uv = (coord.xy - 0.5 * dimensions.xy) / min(dimensions.x, dimensions.y);
    uv *= scale;

    // Slow rotation
    float angle = rotation * time;
    mat2 rot = mat2(cos(angle), -sin(angle), sin(angle), cos(angle));
    uv = rot * uv;

    // Shift to [0,1] for Chladni formula
    vec2 plate = uv + 0.5;

    // Total audio energy for silent fallback
    float totalEnergy = 0.0;
    for (int i = 0; i < 256; i += 8) {
        totalEnergy += audio[i];
    }

    // Accumulate displacement field and per-mode color layers
    float displacement = 0.0;
    float totalAmp = 0.0;
    vec3 colorAccum = vec3(0.0);

    int mc = clamp(modeCount, 2, 12);

    for (int i = 0; i < mc; i++) {
        float n = modes[i].x;
        float m = modes[i].y;

        float amp;
        if (totalEnergy < 0.05) {
            amp = 0.15 + 0.1 * sin(time * 0.5 + float(i) * 1.7);
        } else {
            amp = getModeAmplitude(i, mc) * audioSensitivity;
        }

        totalAmp += amp;

        // Chladni pattern
        float mode = sin(n * PI * plate.x) * sin(m * PI * plate.y)
                   + sin(m * PI * plate.x) * sin(n * PI * plate.y);

        displacement += amp * mode;

        // Per-mode soft glow with unique hue
        float modeDisp = amp * mode;
        // Soft cosine falloff instead of sharp Gaussian
        float softPattern = 0.5 + 0.5 * cos(clamp(modeDisp * sharpness, -PI, PI));
        float modeHue = colorShift + float(i) / float(mc) * 0.6 + time * 0.02;
        float modeSat = 0.4 + 0.3 * (1.0 - softPattern);
        float modeVal = softPattern * amp * brightness;
        colorAccum += hsv2rgb(vec3(modeHue, modeSat, modeVal));
    }

    // Energy scaling
    float energyScale = clamp(totalAmp * 2.0, 0.0, 1.0);

    // Overall soft pattern from combined displacement
    float combined = 0.5 + 0.5 * cos(clamp(displacement * sharpness * 0.5, -PI, PI));

    // Base plasma color from combined field
    float hue = colorShift + combined * 0.3 + sin(time * 0.1) * 0.05;
    float sat = 0.3 + combined * 0.3;
    float val = combined * brightness * 0.6 * energyScale;
    vec3 baseColor = hsv2rgb(vec3(hue, sat, val));

    // Blend per-mode colors with base
    vec3 color = baseColor + colorAccum * 0.5 * energyScale;

    // Decay: blend with previous frame
    vec2 texUv = coord.xy / dimensions.xy;
    vec4 prevColor = texture(tex, texUv);
    color = mix(color, prevColor.rgb, decay);

    outputColor = vec4(color, 1.0);
}
