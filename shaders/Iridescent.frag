#version 150

uniform sampler2D tex;
uniform vec2 dimensions;
uniform float time; // speed-scaled accumulated phase (integrated on the CPU)
uniform float angle;
uniform float layerCount;
uniform float frequency;
uniform float amplitude;
uniform float amplitudeWobble;
uniform float thickness;
uniform float hueShift;
uniform float colorSpread;
uniform float layerHueOffset;
uniform float colorCycleSpeed;
uniform float brightness;
uniform float waveShape;
uniform float harmonics;
uniform float warpAmount;
uniform float warpFreq;
uniform float skew;
uniform float turbulence;
uniform float detune;
in vec2 coord;
out vec4 outputColor;

#define PI 3.14159265

vec3 palette(in float t) {
    vec3 a = vec3(0.5, 0.5, 0.5);
    vec3 b = vec3(0.5, 0.5, 0.5);
    vec3 c = vec3(1.0, 1.0, 1.0);
    vec3 d = vec3(0.1, 0.4, 0.5);
    return a + b * cos(2.0 * PI * (c * t + d));
}

// Morphable oscillator: sine (0) -> triangle (1) -> square (2) -> sawtooth (3)
float waveShapeFn(float p, float morph) {
    float ph = fract(p / (2.0 * PI));
    float s = sin(p);
    float tri = 1.0 - 4.0 * abs(fract(ph + 0.25) - 0.5);
    float sq = sign(s);
    float saw = 2.0 * ph - 1.0;
    float m = clamp(morph, 0.0, 3.0);
    if (m < 1.0) return mix(s, tri, m);
    else if (m < 2.0) return mix(tri, sq, m - 1.0);
    else return mix(sq, saw, m - 2.0);
}

// Cheap value noise for turbulence
float hash11(float p) {
    p = fract(p * 0.1031);
    p *= p + 33.33;
    p *= p + p;
    return fract(p);
}
float noise1(float x) {
    float i = floor(x);
    float f = fract(x);
    float u = f * f * (3.0 - 2.0 * f);
    return mix(hash11(i), hash11(i + 1.0), u) * 2.0 - 1.0;
}

vec4 wave(vec2 uv, float amp, float freq, float phase, float thick, vec3 hue) {
    float x = uv.x - phase;
    float p = freq * x;
    // Warp (FM): bend the wave with a secondary oscillator
    p += warpAmount * sin(warpFreq * x + time);
    // Skew: lean the peaks for an asymmetric, breaking-wave look
    p += skew * sin(p);
    // Morphable shape plus optional harmonic overtones
    float w = waveShapeFn(p, waveShape);
    w += harmonics * 0.5 * waveShapeFn(2.0 * p, waveShape);
    w += harmonics * 0.25 * waveShapeFn(3.0 * p, waveShape);
    w /= (1.0 + 0.75 * harmonics);
    // Turbulence: organic noise jitter
    w += turbulence * noise1(x * 6.0 + time);
    float y = uv.y + amp * w;
    float bright = smoothstep(0.0, 1.0, 1.0 - abs(y) / thick);
    return vec4(vec3(bright) * hue, 1.0);
}

void main()
{
    vec2 uv = (2.0 * coord - dimensions.xy) / dimensions.y;

    // Rotate the wave field by the given angle (degrees -> radians on the CPU)
    float cosA = cos(angle);
    float sinA = sin(angle);
    uv *= mat2(cosA, -sinA, sinA, cosA);

    float t = time;

    vec4 color = vec4(0.0, 0.0, 0.0, 1.0);

    for (float i = 0.0; i < layerCount; i += 1.0) {
        float layer = i / layerCount;
        float amp = amplitude + amplitudeWobble * sin(t + layer) * (1.0 - layer);
        // Detune: spread each layer's frequency so the stack beats against itself
        float freq = frequency * (1.0 + detune * (layer - 0.5));
        float phase = t * (1.0 - layer);
        float thick = thickness + 0.001 * pow(abs(uv.x), 8.0);
        vec3 hue = palette(colorSpread * uv.x + layerHueOffset * layer - colorCycleSpeed * t + hueShift);
        color += wave(uv, amp, freq, phase, thick, hue);
    }

    outputColor = vec4(color.rgb * brightness, 1.0);
}
