#version 150

// Ported from "DiscoSun" by @WorkingClassHacker (CC-BY-NC-SA-4.0)
// based on "Abstract Shine" by @Frostbyte. Raymarched corkscrew tunnel
// with IQ cosine palette, shimmer interference layer and vignette.

uniform sampler2D tex;
uniform vec2 dimensions;
uniform float time;
uniform float speed;      // overall animation speed
uniform float twist;      // corkscrew rotation amount
uniform float shimmer;    // interference / strobe layer strength
uniform float brightness; // exposure

in vec2 coord;
out vec4 outputColor;

// Rotation matrix using cosine phase offsets (compact 2D rotation without sin()).
#define R(a) mat2(cos(a + vec4(0, 33, 11, 0)))

// IQ's continuous cosine palette (MIT) — smooth, periodic color gradients.
vec3 palette(float i) {
    const vec3 a = vec3(0.50, 0.38, 0.26);  // base tone (warm midtone)
    const vec3 b = vec3(0.50, 0.35, 0.25);  // amplitude (vibrance)
    const vec3 c = vec3(1.00);              // frequency (cyclic complexity)
    const vec3 d = vec3(0.00, 0.12, 0.25);  // phase offsets (hue shift)
    return a + b * cos(6.2831853 * (c * i + d));
}

void main() {
    // pixel coordinate
    vec2 u = coord.xy;

    // normalized screen space centered at origin
    vec2 uv = (u - 0.5 * dimensions.xy + 0.5) / dimensions.y;

    // single time source so every knob scales together
    float ts = time * speed;
    float t = mod(ts, 6.283185);

    float i = 0.0, s;
    vec3 p = vec3(0.0);

    vec4 color = vec4(0.0);

    // ray direction through pixel (camera ray)
    vec3 d = normalize(vec3(2.0 * u - dimensions.xy, dimensions.y));

    // starting depth → creates forward motion
    p.z = t;

    // raymarch loop
    for (; i < 20.0; i++) {
        // depth-dependent rotation → corkscrew tunnel motion
        p.xy *= R((-p.z * 0.01 - t * 0.05) * twist);

        // base step size
        s = 0.6;

        // cylindrical confinement → tunnel boundary at radius ≈ 10
        s = max(s, 4.0 * (-length(p.xy) + 10.0));

        // organic deformation field → adds flow & energy patterns
        s += abs(p.y * 0.004 + sin(t - p.x * 0.5) * 0.9 + 1.0);

        // march ray forward
        p += d * s;

        // volumetric glow accumulation
        color += 1.0 / (s * 0.2);
    }

    // apply palette based on final ray distance (depth-dependent coloration)
    color *= vec4(palette(length(p) / (abs(sin(ts * 0.02) * 50.0) + 6.0)), 1.0);

    // time-gated screen-space shimmer / interference layer
    color -= 20.0 * shimmer *
        smoothstep(0.001, abs(sin(ts * 5.0)),
                   0.7 - length(sin(uv * 200.0) / 1.5) - abs(uv.y) + 0.2);

    // brightness normalization (exposure)
    color /= (0.5e2 / brightness);

    // radial gradient
    float l = length(uv);

    // vignette
    color *= 1.2 - l;

    // center glow
    color = mix(color, palette(l - 0.23).rgbr, 1.0 - smoothstep(0.01, 0.95, l));

    // soft highlight compression
    color = tanh(color + color);

    outputColor = vec4(color.rgb, 1.0);
}
