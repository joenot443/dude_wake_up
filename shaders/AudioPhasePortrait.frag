#version 150

uniform sampler2D tex;
uniform vec2 dimensions;
uniform float time;
uniform float audio[256];
uniform float frameDt;
uniform float persistence;
uniform float beamIntensity;
uniform float ribbon;
uniform float portrait;
uniform float kickPunch;
uniform float hueShift;
uniform float audioSensitivity;
uniform float brightness;

in vec2 coord;
out vec4 outputColor;

#define PI 3.14159265359

mat2 rot(float a){ float c = cos(a), s = sin(a); return mat2(c, -s, s, c); }
float hash12(vec2 p){ vec3 p3 = fract(vec3(p.xyx) * 0.1031); p3 += dot(p3, p3.yzx + 33.33); return fract((p3.x + p3.y) * p3.z); }

// Spectrum at a 0..1 frequency position, averaged over neighbours to kill flicker.
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
// Luminance tonemap with a soft film shoulder: keeps hue, rolls off gently.
vec3 tonemap(vec3 c, float b){
    c = max(c, 0.0);
    float L = dot(c, vec3(0.3, 0.59, 0.11));
    float Lt = 1.0 - exp(-L * b);
    vec3 t = c * Lt / max(L, 1e-4);
    return mix(t, vec3(Lt) * vec3(1.0, 0.97, 0.93), smoothstep(0.6, 1.0, Lt) * 0.3);
}

// Audio Phase Portrait — a scope beam tracing the music's figure in 3D, with phosphor memory.
// Geometry is computed CPU-side (see beamPts / figurePts): a harmonograph whose form follows
// the slow spectrum, blended toward a GENUINE delay-embedded phase portrait of the raw waveform
// as the sound becomes tonal (sustained notes lock into closed loops; drums spray them apart).
// The shader only renders: a hot beam with depth of field, string-art rulings to the echo
// shell, an etched graticule lit near the beam head, and phosphor persistence in feedback
// (fresh blue-white -> afterglow -> deep tone). Kicks: a display-only flash (exactly undone the
// next frame, so the feedback never gains), a wider beam, and a one-frame strobe of the closed figure.

uniform vec3 beamPts[58];       // pen A: 0..28 tail->head, pen B (echo shell): 29..57   (x, y, depth)
uniform vec3 figurePts[96];     // the whole closed figure, drawn only on the strobe frame
uniform vec3 embedPts[96];      // ONE triggered period of the raw history, delay-embedded (genuine phase portrait)
uniform float portraitGain;
uniform float figureGain;      // CPU: dips to 0 while the figure swaps shape (a dissolve, never a sweep)     // CPU: periodicity (autocorr peak) x loop-closure quality
uniform float kickEnv, strobe;
uniform float flashNow, flashPrev;   // display-flash amount this instance applies now / applied last frame

const int NB = 28;

float segD(vec2 p, vec2 a, vec2 b, out float h){
    vec2 pa = p - a, ba = b - a;
    h = clamp(dot(pa, ba) / max(dot(ba, ba), 1e-8), 0.0, 1.0);
    return length(pa - ba * h);
}
float luma(vec3 c){ return dot(c, vec3(0.3, 0.59, 0.11)); }
// Display-only kick flash that survives 8-bit storage: f(x) = x + a x^2 (1-x) maps [0,1] onto [0,1]
// monotonically (a <= 1) and keeps black black (no milky lift); flashInv undoes it (2 Newton steps).
vec3 flashF(vec3 x, float a){ x = clamp(x, 0.0, 1.0); return x + a * x * x * (1.0 - x); }   // black stays black
vec3 flashInv(vec3 y, float a){
    if (a < 1e-4) return y;
    y = clamp(y, 0.0, 1.0);
    vec3 x = y;
    for (int i = 0; i < 2; i++){                       // Newton: f(x) - y = 0, f'(x) = 1 + a(2x - 3x^2)
        vec3 fx = x + a * x * x * (1.0 - x) - y;
        x = clamp(x - fx / (1.0 + a * (2.0 * x - 3.0 * x * x)), 0.0, 1.0);
    }
    return x;
}

vec3 phosFresh(float e){ return e < 1.0 ? vec3(0.78, 0.88, 1.00) : (e < 2.0 ? vec3(1.00, 0.86, 0.62) : vec3(0.88, 0.95, 1.00)); }
vec3 phosAfter(float e){ return e < 1.0 ? vec3(0.55, 0.78, 0.30) : (e < 2.0 ? vec3(1.00, 0.52, 0.12) : vec3(0.30, 0.72, 0.70)); }
vec3 phosDeep(float e){  return e < 1.0 ? vec3(0.08, 0.20, 0.10) : (e < 2.0 ? vec3(0.45, 0.12, 0.05) : vec3(0.05, 0.16, 0.24)); }

// one beam polyline (9 sub-segments' worth of loops kept small): returns core/halo energy
void beamLine(vec2 uv, vec3 a, vec3 b, float wMul, inout float core, inout float halo, inout float zAcc){
    float h;
    float len = length(b.xy - a.xy);
    if (len > 0.3) return;                                    // beam blanking
    float d = segD(uv, a.xy, b.xy, h);
    float z = mix(a.z, b.z, h);
    float wc = 0.005 * (1.0 + 1.2 * abs(z)) * wMul;           // depth of field
    float hot = clamp(0.010 / (len + 0.002), 0.0, 1.4);       // slow beam burns hotter (capped)
    float fog = exp(-max(z, 0.0) * 1.3) * (1.0 + 0.6 * max(-z, 0.0));
    float c = exp(-(d * d) / (wc * wc)) * (0.005 / wc) * hot * fog;
    core = max(core, c);   // max, not sum: no beading at sub-segment joints
    halo = max(halo, (exp(-d / 0.035) * 0.016 + exp(-d / 0.08) * 0.002) * hot * fog);   // max: no fat summed tubes
    zAcc += c * z;
}

// the steady figure trace: constant brightness along its length (no slow-beam burn), depth-of-field width
void traceLine(vec2 uv, vec3 a, vec3 b, float wMul, inout float core, inout float halo){
    float h;
    float d = segD(uv, a.xy, b.xy, h);
    float z = mix(a.z, b.z, h);
    float wc = 0.0035 * (1.0 + 1.0 * abs(z)) * wMul;
    float fog = exp(-max(z, 0.0) * 1.1) * (1.0 + 0.4 * max(-z, 0.0));
    core = max(core, exp(-(d * d) / (wc * wc)) * (0.0035 / wc) * fog);
    halo = max(halo, (exp(-d / 0.02) * 0.10 + exp(-d / 0.07) * 0.02) * fog);
}

void main(){
    vec2 res = dimensions;
    vec2 texUv = coord / res;
    vec2 pc = vec2(coord.x, res.y - coord.y);
    vec2 uv = (2.0 * pc - res) / res.y;

    float e = totalEnergy();
    float live = smoothstep(0.05, 0.4, e);
    float fs = clamp(frameDt * 30.0, 0.25, 3.0);
    float phos = mod(floor(time / 20.0), 3.0);
    vec3 cF = phosFresh(phos), cA = phosAfter(phos), cD = phosDeep(phos);
    // echo shell: the afterglow's own family, deeper (amber -> oxblood, P7 -> bottle green, teal -> deep sea)
    vec3 cB = cA * (phos < 1.0 ? vec3(0.45, 0.70, 0.60) : (phos < 2.0 ? vec3(0.75, 0.45, 0.35) : vec3(0.45, 0.60, 0.85)));

    float wMul = 1.0 + 1.2 * kickEnv * kickPunch;
    vec3 fresh = vec3(0.0);

    // --- pen A (the figure) and pen B (echo shell), with bounding-box early-out ---
    for (int pen = 0; pen < 2; pen++){
        int o = pen * (NB + 1);
        vec2 bmin = vec2(1e9), bmax = vec2(-1e9);
        for (int j = 0; j <= NB; j++){ bmin = min(bmin, beamPts[o + j].xy); bmax = max(bmax, beamPts[o + j].xy); }
        if (pen == 1) for (int j = 0; j <= NB; j += 4){ bmin = min(bmin, beamPts[j].xy); bmax = max(bmax, beamPts[j].xy); }
        if (any(lessThan(uv, bmin - 0.3)) || any(greaterThan(uv, bmax + 0.3))) continue;
        float core = 0.0, halo = 0.0, zAcc = 0.0;
        for (int j = 0; j < NB; j++) beamLine(uv, beamPts[o + j], beamPts[o + j + 1], wMul, core, halo, zAcc);
        if (pen == 0){
            fresh += (vec3(1.0) * core * 1.2 + cF * (core * 0.8 + halo)) * 1.6;
        } else {
            fresh += cB * (core * 0.7 + halo * 0.8) * 0.8;
            // string-art rulings between the figure and its echo shell
            float rw = max(0.0015, 3.0 / res.y);
            for (int j = 0; j <= NB; j += 4){
                if (length(beamPts[o + j].xy - beamPts[j].xy) > 0.45) continue;    // long spokes would persist outside
                float h;
                float d = segD(uv, beamPts[j].xy, beamPts[o + j].xy, h);
                fresh += cB * exp(-(d * d) / (rw * rw)) * 0.2 * ribbon;
            }
        }
    }
    // --- the whole closed figure, redrawn every frame as a steady afterglow trace (so the form is always
    //     legible whatever the beam speed); kicks brighten it and the strobe frame flashes it white ---
    {
        vec2 fmin = vec2(1e9), fmax = vec2(-1e9);
        for (int j = 0; j < 96; j += 3){ fmin = min(fmin, figurePts[j].xy); fmax = max(fmax, figurePts[j].xy); }
        float core = 0.0, halo = 0.0;
        if (all(greaterThan(uv, fmin - 0.2)) && all(lessThan(uv, fmax + 0.2)))
            for (int j = 0; j < 95; j++) traceLine(uv, figurePts[j], figurePts[j + 1], 1.0 + 0.6 * strobe, core, halo);
        float glow = (0.75 + 0.3 * kickEnv * kickPunch) * figureGain;
        fresh += (cA * (core * 0.9 + halo) + vec3(0.25) * core) * glow;
        fresh += (vec3(1.0) * core * 0.4 + cF * (core * 0.6 + halo * 0.6)) * 0.5 * kickPunch * strobe * figureGain;
    }
    // --- the genuine phase portrait of the sound, inside the figure: fades in with tonality ---
    float tonal = portraitGain * portrait;
    if (tonal > 0.01){
        vec2 emin = vec2(1e9), emax = vec2(-1e9);
        for (int j = 0; j < 96; j += 4){ emin = min(emin, embedPts[j].xy); emax = max(emax, embedPts[j].xy); }
        if (all(greaterThan(uv, emin - 0.25)) && all(lessThan(uv, emax + 0.25))){
            float core = 0.0, halo = 0.0, zAcc = 0.0;
            for (int j = 0; j < 95; j++) beamLine(uv, embedPts[j], embedPts[j + 1], 0.8, core, halo, zAcc);
            fresh += (vec3(1.0) * core * 0.6 + cF * (core * 0.7 + halo * 0.7)) * 0.75 * tonal;
        }
    }
    fresh *= beamIntensity * (0.85 + 0.45 * kickEnv * kickPunch);   // new light on the kick (not feedback gain)

    // --- graticule: 10 x 8 divisions with minor ticks, etched faintly, lit near the beam head ---
    vec2 head = beamPts[NB].xy;
    float asp = res.x / res.y;
    vec2 g = vec2((uv.x / asp) * 5.0, uv.y * 4.0);                 // 10 x 8 divisions over the frame
    vec2 gf = abs(fract(g) - 0.5);
    float major = exp(-min(gf.x * res.x / 10.0, gf.y * res.y / 8.0) * 1.2);
    vec2 tk = abs(fract(g * 5.0) - 0.5);
    float minor = exp(-abs(uv.x) * res.y * 0.6) * step(tk.y, 0.12) + exp(-abs(uv.y) * res.y * 0.6) * step(tk.x, 0.12);
    float lightAtHead = 0.05 + 0.12 * exp(-length(uv - head) * 1.2);
    vec3 grat = mix(vec3(0.6, 0.75, 0.8), cA, 0.4) * (major * 0.7 + minor * 0.5) * lightAtHead;

    float haze = 0.55 + 0.45 * dot_fbm(vec3(uv * 2.2, time * 0.06));
    vec3 lit = cF * 0.010 / (0.08 + length(uv - head)) * haze;
    vec3 space = mix(vec3(0.004, 0.006, 0.014), vec3(0.012, 0.010, 0.022), smoothstep(-1.0, 1.0, uv.y));

    // fresh light: vignette + silence dimming apply HERE only (never to the fed-back state)
    float vig = smoothstep(2.0, 0.6, length(uv * vec2(0.75, 1.0)));
    vec3 now = tonemap(hueRotate(fresh + lit, hueShift * 6.2831853), brightness);
    now = (now + grat + space) * mix(1.0, vig, 0.3) * (0.75 + 0.25 * live);

    // --- phosphor persistence ---
    vec2 px = 0.3 / res;
    vec3 prev = 0.25 * (texture(tex, texUv + vec2(px.x, 0.0)).rgb + texture(tex, texUv - vec2(px.x, 0.0)).rgb
                      + texture(tex, texUv + vec2(0.0, px.y)).rgb + texture(tex, texUv - vec2(0.0, px.y)).rgb);
    prev = flashInv(prev, flashPrev);                               // undo the flash THIS instance applied last frame
    float L = luma(prev);
    prev = mix(prev, cA * (L / luma(cA)), 1.0 - pow(0.93, fs));                               // -> afterglow
    prev = mix(prev, cD * (L / luma(cD)), (1.0 - pow(0.90, fs)) * smoothstep(0.25, 0.0, L) * smoothstep(0.004, 0.03, L));  // dim strokes -> deep tone (not the floor)
    // minutes-scale arc: every 7th 20 s epoch (~2.3 min) its stable middle (ratios not morphing) becomes a
    // 'long exposure': persistence climbs to 0.997 and the rotating figure builds a dense time-exposure
    float eidL = floor(time / 20.0), efL = fract(time / 20.0);
    float longExp = (mod(eidL, 7.0) == 3.0 ? 1.0 : 0.0) * smoothstep(0.05, 0.15, efL) * (1.0 - smoothstep(0.62, 0.74, efL));
    // during each epoch's ratio morph, fade fast so the transition is a short liquid trail, not a tangle
    float morphing = smoothstep(0.80, 0.84, efL) * (1.0 - smoothstep(0.92, 0.96, efL));
    float keep = mix(mix(mix(0.93, 0.995, persistence), 0.997, longExp), 0.90, morphing);
    // 8-bit safe decay: multiplicative + a dithered subtractive floor, so phosphor always reaches true black
    float floorStep = (0.75 + 0.5 * hash12(coord + fract(time) * 91.0)) / 255.0 * fs * (1.0 - 0.75 * longExp);
    prev = max(prev * pow(keep, fs) - floorStep, 0.0);

    vec3 col = max(now, prev);
    col = flashF(col, flashNow);                                   // display-only flash (undone next frame)
    outputColor = vec4(col, 1.0);
}
