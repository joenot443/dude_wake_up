#version 150

uniform sampler2D tex;
uniform vec2 dimensions;
uniform float time;
uniform float audio[256];

uniform float scale;
uniform float ripple;
uniform float sheen;
uniform float gust;
uniform float hueShift;
uniform float audioSensitivity;
uniform float brightness;
uniform float kickLevel;   // CPU: relative bass onset envelope (0..1)
uniform float midLevel;    // CPU: mids against their own 8 s level (0.5 = typical)
uniform float highLevel;   // CPU: highs against their own 8 s level

in vec2 coord;
out vec4 outputColor;

#define PI 3.14159265359

mat2 rot(float a){ float c = cos(a), s = sin(a); return mat2(c, -s, s, c); }
float hash12(vec2 p){ vec3 p3 = fract(vec3(p.xyx) * 0.1031); p3 += dot(p3, p3.yzx + 33.33); return fract((p3.x + p3.y) * p3.z); }

// Sample the spectrum at a 0..1 frequency position, averaging neighbours to kill flicker.
float spec(float f){
    float x = clamp(f, 0.0, 1.0) * 255.0;
    int i = int(x);
    float s = 0.0;
    for (int j = -2; j <= 2; j++) s += audio[clamp(i + j, 0, 255)];
    return s * 0.2;
}
// Mean amplitude across a frequency range [lo, hi] (fractions of the spectrum).
float band(float lo, float hi){
    float s = 0.0;
    for (int j = 0; j < 8; j++) s += audio[clamp(int(mix(lo, hi, float(j) / 7.0) * 255.0), 0, 255)];
    return s / 8.0;
}
float totalEnergy(){
    float e = 0.0;
    for (int i = 0; i < 256; i += 8) e += audio[i];
    return e;
}
float dot_noise(vec3 p){
    const float PHI = 1.618033988;
    const mat3 GOLD = mat3(-0.571464913, 0.814921382, 0.096597072,
                           -0.278044873, -0.303026659, 0.911518454,
                            0.772087367, 0.494042493, 0.399753815);
    return dot(cos(GOLD * p), sin(PHI * p * GOLD));
}
float dot_fbm(vec3 p){
    float v = 0.0, a = 0.5;
    for (int i = 0; i < 5; i++){ v += a * dot_noise(p) / 3.0; p *= 1.9; a *= 0.5; }
    return v;
}

vec3 hueRotate(vec3 c, float a){
    const vec3 k = vec3(0.57735);
    float ca = cos(a);
    return c * ca + cross(k, c) * sin(a) + k * dot(k, c) * (1.0 - ca);
}

// Luminance tonemap with a film-like highlight shoulder: keeps hue, rolls off softly.
vec3 tonemap(vec3 c, float b){
    c = max(c, 0.0);
    float L = dot(c, vec3(0.3, 0.59, 0.11));
    float Lt = 1.0 - exp(-L * b);
    vec3 t = c * Lt / max(L, 1e-4);
    return mix(t, vec3(Lt) * vec3(1.0, 0.97, 0.93), smoothstep(0.6, 1.0, Lt) * 0.25);
}

// Audio Silk — draped silk lit like a studio product shot.
// Long flowing drape folds (low-octave turbulence) carry fine wrinkles; a warm key light
// orbits slowly and a cool rim light opposes it, so anisotropic (Kajiya-Kay) sheen bands
// glide along the folds. The music is woven in: every fold crest is assigned a frequency
// band and glows with it. Kicks gust through the cloth (billow + sheen flash), highs
// scatter glints along the highlights, and designed palettes crossfade every ~16 s.

// Three restrained dye palettes: shadow, body A, body B, sheen.
void paletteAt(int i, out vec3 shadow, out vec3 bodyA, out vec3 bodyB, out vec3 sh){
    if (i == 0){        // ink / teal / plum, gold sheen
        shadow = vec3(0.010, 0.015, 0.040); bodyA = vec3(0.04, 0.36, 0.42); bodyB = vec3(0.34, 0.08, 0.30); sh = vec3(1.00, 0.80, 0.50);
    } else if (i == 1){ // oxblood / rose, champagne sheen
        shadow = vec3(0.030, 0.008, 0.012); bodyA = vec3(0.26, 0.03, 0.06); bodyB = vec3(0.38, 0.10, 0.14); sh = vec3(1.00, 0.88, 0.70);
    } else {            // midnight / sapphire, silver-blue sheen
        shadow = vec3(0.006, 0.010, 0.030); bodyA = vec3(0.04, 0.10, 0.34); bodyB = vec3(0.10, 0.28, 0.50); sh = vec3(0.78, 0.84, 0.92);
    }
}

vec2 drapeWarp(vec2 p, float t, float amp){
    float freq = 1.1;
    mat2 r = mat2(0.6, -0.8, 0.8, 0.6);
    for (int i = 0; i < 5; i++){
        float ph = freq * (p * r).y + t * (0.5 + 0.15 * float(i)) + float(i) * 1.7;
        p += amp * r[0] * sin(ph) / freq * (i < 2 ? 1.0 : 0.35);   // taper upper octaves: hang, don't curl
        r *= mat2(0.6, -0.8, 0.8, 0.6);
        freq *= 1.6;
    }
    return p;
}

// Fabric: returns height (0..1) and the main fold phase (for crest indexing).
float fabric(vec2 p, float t, float amp, float billow, out float phase){
    vec2 q = drapeWarp(p * vec2(1.0, 0.7), t, amp);              // gravity: folds hang lengthwise
    phase = q.y * 3.6 + 0.7 * sin(q.x * 0.9 + t * 0.35) + 0.5 * sin(q.x * 2.3 - q.y * 1.1 + t * 0.5) + 0.25 * sin(q.x * 5.1 + q.y * 3.7 - t * 0.4) + billow;
    float h = 1.0 - pow(1.0 - (0.5 + 0.5 * sin(phase)), 2.2);   // soft crowns, tight creases
    // fine wrinkles riding on the drape, strongest on the slopes
    float wr = sin(q.y * 13.0 + q.x * 4.0 + t * 0.8) * 0.5 + sin(q.x * 9.0 - q.y * 7.0 - t * 0.6) * 0.5;
    h += 0.12 * wr * 4.0 * h * (1.0 - h);       // smooth weight: abs() here kinks the normals and tears the sheen
    return h;
}

void main(){
    vec2 res = dimensions;
    vec2 pc = vec2(coord.x, res.y - coord.y);          // up-is-up (engine coord.y runs down)
    vec2 sUv = pc / res;
    vec2 uv = (2.0 * pc - res) / res.y;

    float e = totalEnergy();
    float live = smoothstep(0.05, 0.4, e);
    float sens = audioSensitivity;
    // kick / mid / high come from the CPU (relative onsets and auto-gained levels): absolute
    // thresholds on the spectrum pin on loud masters, where the low and mid bins clamp at 1
    float kick = (0.3 + 0.7 * kickLevel) * live;   // a resting gust, so the cloth keeps its sheen between beats
    float mid  = mix(0.3, 0.2 + 0.9 * midLevel, live);
    float high = mix(0.1, 0.1 + 0.9 * highLevel, live);

    // slow camera: drift, gentle roll, breathing zoom
    float t = time;
    vec2 p = uv * rot(0.25 * sin(t * 0.021) + 0.35) * (scale * (1.0 + 0.12 * sin(t * 0.017)));
    p += vec2(t * 0.05, 0.08 * sin(t * 0.031));

    // gust: kicks shove the cloth along a slowly turning wind and swell the folds
    vec2 gustDir = vec2(cos(t * 0.05), sin(t * 0.05));
    float amp = ripple * (1.0 + 0.35 * kick * gust);
    float billow = kick * gust * 0.5 * sin(dot(p, gustDir) * 1.3 - t * 1.5);

    float eps = 1.5 / res.y;
    float ph, phx, phy;
    float h  = fabric(p, t, amp, billow, ph);
    float hx = fabric(p + vec2(eps, 0.0), t, amp, billow, phx);
    float hy = fabric(p + vec2(0.0, eps), t, amp, billow, phy);
    vec2 grad = vec2(hx - h, hy - h) / eps;
    float relief = 0.24;                                 // fold depth for lighting
    vec3 N = normalize(vec3(-grad * relief, 1.0));
    // fold direction from the fold-phase gradient (never vanishes, unlike the height's at crests)
    // fibres run ACROSS the fold (along the fold-phase gradient, which never vanishes) and
    // tilt with the slope, so Kajiya-Kay draws thin bright lines riding each fold flank
    vec2 pg = vec2(phx - ph, phy - ph);
    vec2 across = normalize(pg + 1e-6);
    vec3 Tng = normalize(vec3(across, dot(grad, across) * relief));
    vec3 V = vec3(0.0, 0.0, 1.0);

    // lights: warm key orbiting slowly, cool rim opposite and low
    float ka = t * 0.11 + 0.8;
    vec3 L1 = normalize(vec3(cos(ka), sin(ka), 0.75));
    vec3 L2 = normalize(vec3(-cos(ka) * 0.9, -sin(ka) * 0.9, 0.35));

    // palette epochs: crossfade every ~16 s
    float pe = t / 16.0;
    int pi0 = int(mod(floor(pe), 3.0));
    int pi1 = int(mod(floor(pe) + 1.0, 3.0));
    // dye bleed: the next palette spreads through the cloth along a ragged front
    float front = fract(pe) * 1.5 - 0.25;                         // sweeps across in the last ~third
    float bleedN = 0.5 + 0.5 * dot_fbm(vec3(p * 0.8, floor(pe) * 3.1));
    float wipe = sUv.x * 0.6 + sUv.y * 0.25 + bleedN * 0.45 - 0.2;
    float pf = smoothstep(wipe - 0.04, wipe + 0.04, (front - 0.75) * 4.0);
    vec3 sh0, a0, b0, s0, sh1, a1, b1, s1;
    paletteAt(pi0, sh0, a0, b0, s0);
    paletteAt(pi1, sh1, a1, b1, s1);
    vec3 shadowC = mix(sh0, sh1, pf), bodyA = mix(a0, a1, pf), bodyB = mix(b0, b1, pf), sheenC = mix(s0, s1, pf);

    // body colour drifts across the cloth between the two dyes
    float dye = smoothstep(-0.6, 0.6, sin(p.x * 0.6 + p.y * 0.25 + t * 0.07) + 0.3 * sin(ph * 0.5));
    vec3 body = mix(bodyA, bodyB, dye);

    float ao = mix(0.22, 1.0, smoothstep(0.0, 0.7, h));        // valleys sink
    float d1 = max((dot(N, L1) + 0.35) / 1.35, 0.0);            // wrapped diffuse
    float d2 = max(dot(N, L2), 0.0);

    // anisotropic sheen (Kajiya-Kay) — long bands along each fold
    vec3 H1 = normalize(L1 + V), H2 = normalize(L2 + V);
    float th1 = dot(Tng, H1), th2 = dot(Tng, H2);
    // Gaussian lobes widened by their own pixel footprint so thin lines dim instead of crawling
    float sig1 = 0.075 + 0.9 * fwidth(th1);
    float sig2 = 0.10 + 0.9 * fwidth(th2);
    float kk1 = exp(-th1 * th1 / (2.0 * sig1 * sig1)) * (0.075 / sig1) * smoothstep(0.0, 0.3, dot(N, L1));
    float kk2 = exp(-th2 * th2 / (2.0 * sig2 * sig2)) * (0.10 / sig2) * smoothstep(0.0, 0.3, dot(N, L2));

    vec3 col = shadowC * ao + body * d1 * ao * 0.9;
    col += body * 0.6 * mix(vec3(1.0), vec3(0.6, 0.8, 1.0), 0.5) * d2 * 0.35 * ao;   // cool rim fill
    vec3 sheenTint = mix(sheenC, mix(bodyB, bodyA, dye) * 2.5, 0.3);   // shot silk keeps colour in its sheen
    col += sheenTint * kk1 * sheen * (0.55 + 0.6 * mid) * ao;
    col += mix(sheenTint, vec3(0.6, 0.85, 1.0), 0.5) * kk2 * sheen * 0.35 * ao;

    // the music woven in: each crest belongs to a band and glows with it
    float k = floor((ph - 1.5707963) / 6.2831853 + 0.5);
    float f = clamp(0.5 + 0.5 * sin(k * 0.45 + t * 0.05), 0.0, 1.0);   // neighbouring folds = neighbouring bands
    float lvl = pow(clamp(spec(pow(f, 1.3) * 0.85) * sens, 0.0, 1.5), 0.6);
    lvl = mix(0.3 + 0.2 * sin(k * 1.7 + t * 0.6), lvl, live);
    float crest = pow(max(sin(ph - 0.6), 0.0), 40.0) * smoothstep(0.5, 0.95, h);
    vec3 threadTint = mix(sheenC, mix(bodyA, bodyB, 1.0 - dye) * 1.8 + 0.05, 0.5);
    col += threadTint * crest * smoothstep(0.2, 0.7, lvl) * 2.2 * (1.0 - smoothstep(0.1, 0.4, kk1));

    // highs: sparse glints catching the sheen
    vec2 gq = (rot(atan(across.y, across.x)) * (pc / res.y)) * vec2(30.0, 220.0);
    vec2 gid = floor(gq);
    float gh = hash12(gid + floor(t * 6.0) * 0.37);
    float glint = smoothstep(0.975, 1.0, gh) * exp(-abs(fract(gq.y) - 0.5) * 14.0) * exp(-abs(fract(gq.x) - 0.5) * 3.0);
    col += sheenC * glint * smoothstep(0.4, 0.9, kk1) * high * 1.5;

    // kicks: a gust of light sweeps across the sheen, then a contrast punch that reads at distance
    float sweep = smoothstep(0.25, 0.0, abs(fract(dot(uv, gustDir) * 0.35 - t * 0.5) - 0.5));
    col += sheenTint * (kk1 + 0.3 * kk2) * kick * gust * (0.8 + 1.5 * sweep) * ao;
    float Lc = dot(col, vec3(0.3, 0.59, 0.11));
    col = mix(col, col * col * 1.8, 0.2 * kick * smoothstep(0.35, 0.8, Lc));

    // a soft pool of key light gives the frame a lit side and a falling-away side
    col *= 0.75 + 0.4 * smoothstep(1.6, 0.0, length(uv - 0.6 * vec2(cos(ka), sin(ka))));

    // grade: vignette with edge desaturation for depth
    float vig = smoothstep(1.75, 0.35, length(uv * vec2(0.72, 1.0)));
    col = mix(vec3(dot(col, vec3(0.3, 0.59, 0.11))), col, 0.75 + 0.25 * vig);
    col *= 0.6 + 0.4 * vig;

    col = tonemap(hueRotate(col, hueShift * 6.2831853), brightness);
    col *= 0.55 + 0.45 * live;
    outputColor = vec4(col, 1.0);
}
