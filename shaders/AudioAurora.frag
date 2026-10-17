#version 150

uniform sampler2D tex;
uniform vec2 dimensions;
uniform float time;

uniform int ribbonCount;
uniform float waveSpeed;
uniform float height;
uniform float thickness;
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

// Average several bins across a frequency band to prevent flicker
float getBandAmplitude(int bandIndex, int totalBands) {
    float bandStart = float(bandIndex) / float(totalBands);
    float bandEnd = float(bandIndex + 1) / float(totalBands);
    float sum = 0.0;
    for (int j = 0; j < 4; j++) {
        float t = mix(bandStart, bandEnd, float(j) / 3.0);
        int idx = clamp(int(t * 255.0), 0, 255);
        sum += audio[idx];
    }
    return sum * 0.25;
}

// Layered noise for organic ribbon movement
float hash(float n) { return fract(sin(n) * 43758.5453); }

float noise(float x) {
    float i = floor(x);
    float f = fract(x);
    f = f * f * (3.0 - 2.0 * f);
    return mix(hash(i), hash(i + 1.0), f);
}

// Compute the y-position of a ribbon at a given x
float ribbonY(float x, int index, float amp, float t) {
    float seed = float(index) * 7.31;
    float baseY = 0.2 + height * (float(index) + 0.5) / float(ribbonCount);

    // Multiple sine harmonics for organic flow
    float wave = 0.0;
    wave += sin(x * 1.5 + t * waveSpeed * 0.7 + seed) * 0.12;
    wave += sin(x * 3.0 - t * waveSpeed * 1.1 + seed * 2.0) * 0.06;
    wave += sin(x * 5.5 + t * waveSpeed * 0.5 + seed * 0.5) * 0.03;

    // Noise-based displacement for irregularity
    wave += (noise(x * 2.0 + t * waveSpeed * 0.3 + seed) - 0.5) * 0.08;

    // Audio drives vertical displacement
    wave *= (0.3 + amp * 1.5);

    return baseY + wave;
}

void main() {
    vec2 uv = coord.xy / dimensions.xy;
    vec2 texUv = uv;

    // Total audio energy for silent fallback
    float totalEnergy = 0.0;
    for (int i = 0; i < 256; i += 8) {
        totalEnergy += audio[i];
    }

    int rc = clamp(ribbonCount, 2, 10);
    float t = time;

    vec3 color = vec3(0.0);
    float totalAmp = 0.0;

    for (int i = 0; i < rc; i++) {
        // Get audio amplitude for this ribbon's frequency band
        float amp;
        if (totalEnergy < 0.05) {
            // Silent fallback: gentle pulsing
            amp = 0.2 + 0.15 * sin(t * 0.4 + float(i) * 1.3);
        } else {
            // Perceptual curve: the raw magnitude spectrum is tiny except at high
            // volume, so a linear mapping leaves the aurora faint. Raising it to a
            // fractional power lifts quiet/moderate signals toward visibility while
            // still letting loud peaks reach the top of the range.
            float band = getBandAmplitude(i, rc);
            amp = pow(band, 0.45) * audioSensitivity;
        }
        totalAmp += amp;

        // Ribbon y-position at this x
        float ry = ribbonY(uv.x * 6.0, i, amp, t);

        // Distance from pixel to ribbon center
        float dist = abs(uv.y - ry);

        // Ribbon brightness: Gaussian falloff from center
        // Thickness modulated by audio
        float ribbonThick = thickness * (0.5 + amp * 1.0);
        float glow = exp(-dist * dist / (ribbonThick * ribbonThick + 0.0001));

        // Fade at horizontal edges for curtain feel
        float edgeFade = smoothstep(0.0, 0.15, uv.x) * smoothstep(1.0, 0.85, uv.x);
        glow *= edgeFade;

        // Vertical fade: aurora is brighter at its base, fades upward
        float vertFade = smoothstep(0.0, 0.1, uv.y) * smoothstep(1.0, 0.6, uv.y);
        glow *= vertFade;

        // Color: aurora greens/teals for bass, purples/magentas for treble
        // Base palette: 0.3 (green) -> 0.5 (cyan) -> 0.75 (purple) -> 0.85 (magenta)
        float hue = colorShift + 0.3 + float(i) / float(rc) * 0.55;

        // Subtle hue variation along the ribbon
        hue += sin(uv.x * 4.0 + t * 0.3 + float(i)) * 0.05;

        float sat = 0.6 + amp * 0.3;
        float val = glow * brightness * amp;

        vec3 ribbonColor = hsv2rgb(vec3(hue, sat, 1.0)) * val;
        color += ribbonColor;
    }

    // Scale by total energy so silence = dark. sqrt() ramps the scale up quickly
    // from low energy so the aurora reaches full strength well before peak loudness,
    // instead of staying dim until the signal is very loud (which made the overall
    // response roughly quadratic).
    float energyScale = sqrt(clamp(totalAmp * 2.0 / float(rc), 0.0, 1.0));
    color *= energyScale;

    // Subtle vertical shimmer (like real aurora)
    float shimmer = 1.0 + 0.05 * sin(uv.y * 80.0 + t * 3.0);
    color *= shimmer;

    // Decay: blend with previous frame for trailing
    vec4 prevColor = texture(tex, texUv);
    color = mix(color, prevColor.rgb, decay);

    outputColor = vec4(color, 1.0);
}
