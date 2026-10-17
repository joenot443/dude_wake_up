#version 150

uniform sampler2D tex;
uniform vec2 dimensions;
uniform float time;
uniform float audio[256];
uniform float frameDt;
uniform float bassAvg;
uniform float beamPower;
uniform float dispersion;
uniform float haze;
uniform float glass;
uniform float floorGloss;
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

// Audio Prism — an optical bench that turns the audio spectrum into a light spectrum.
// A lamp on the floor throws a white beam up into a prism seated on a black pedestal;
// the glass disperses it (Snell's law, exaggerated dispersion) and the fan crosses the
// hazy room to land on a matte card as a live spectrograph: each wavelength is a
// frequency band (red = bass ... violet = treble, tilt-compensated), quiet bands become
// dark absorption lines. The fan starts white where the colours overlap and separates
// with distance; its volume is light revealed by smoke. Kicks open the fan and breathe
// the room; every ~12 s an epoch may swing the beam through minimum deviation (the fan
// folds back) or eclipse it (collapsing to a white line, blooming back).

const float FLOOR_Y = -0.62;
const float PED_TOP = -0.18;
const float CARD_X = 0.86;
const float CARD_W = 0.38;
const float CARD_TOP = 0.07;     // fits the fan at rest; kicks and swings stay on the card
const int K = 7;
const float AIM_LO = 0.15;
const float AIM_HI = 0.70;
const vec3 WHITE = vec3(1.0, 0.97, 0.92);

vec3 wavelengthColor(float w){      // 0 = red ... 1 = violet, film-like
    vec3 c0 = vec3(0.95, 0.10, 0.04), c1 = vec3(1.00, 0.45, 0.06), c2 = vec3(0.85, 0.85, 0.10);
    vec3 c3 = vec3(0.15, 0.85, 0.35), c4 = vec3(0.08, 0.60, 0.95), c5 = vec3(0.18, 0.22, 1.00), c6 = vec3(0.32, 0.10, 0.70);
    float x = clamp(w, 0.0, 1.0) * 6.0;
    vec3 c = x < 1.0 ? mix(c0, c1, x) : x < 2.0 ? mix(c1, c2, x - 1.0) : x < 3.0 ? mix(c2, c3, x - 2.0)
           : x < 4.0 ? mix(c3, c4, x - 3.0) : x < 5.0 ? mix(c4, c5, x - 4.0) : mix(c5, c6, x - 5.0);
    return c * (1.0 - 0.45 * smoothstep(0.8, 1.0, w));
}
float segDist(vec2 p, vec2 a, vec2 b, out float h){
    vec2 pa = p - a, ba = b - a;
    h = clamp(dot(pa, ba) / dot(ba, ba), 0.0, 1.0);
    return length(pa - ba * h);
}
float raySeg(vec2 o, vec2 d, vec2 a, vec2 b){
    vec2 e = b - a;
    float den = d.x * e.y - d.y * e.x;
    if (abs(den) < 1e-6) return 1e9;
    vec2 ao = a - o;
    float t = (ao.x * e.y - ao.y * e.x) / den;
    float u = (ao.x * d.y - ao.y * d.x) / den;
    return (t > 1e-4 && u >= 0.0 && u <= 1.0) ? t : 1e9;
}

vec2 V[3]; vec2 NRM[3]; vec2 CEN;
void prismSetup(vec2 c, float r){
    CEN = c;
    for (int k = 0; k < 3; k++){
        float a = float(k) * 2.0943951;
        V[k] = c + r * vec2(sin(a), cos(a));
    }
    for (int k = 0; k < 3; k++){
        vec2 a = V[k], b = V[(k + 1) % 3];
        vec2 e = normalize(b - a);
        vec2 n = vec2(e.y, -e.x);
        if (dot(n, (a + b) * 0.5 - c) < 0.0) n = -n;
        NRM[k] = n;
    }
}
float hitPrism(vec2 o, vec2 d, int skip, out vec2 nOut, out int edge){
    float best = 1e9; edge = -1; nOut = vec2(0.0);
    for (int k = 0; k < 3; k++){
        if (k == skip) continue;
        float t = raySeg(o, d, V[k], V[(k + 1) % 3]);
        if (t < best){ best = t; edge = k; nOut = NRM[k]; }
    }
    return best;
}
float sdTri(vec2 p){
    float d = -1e9;
    for (int k = 0; k < 3; k++) d = max(d, dot(p - V[k], NRM[k]));
    return d;
}
float sdBox(vec2 p, vec2 lo, vec2 hi){
    vec2 c = (lo + hi) * 0.5, h = (hi - lo) * 0.5;
    vec2 q = abs(p - c) - h;
    return length(max(q, 0.0)) + min(max(q.x, q.y), 0.0);
}

// ---- traced optics (same for every pixel) ----
vec2 gLamp, gP1, gE, gD0, gD1m, gN2m, gP2m;
vec3 gFanAvg;
int gE2m;
float gPhi[K];
vec2 gP2[K];
bool gOk;
float gLvlLive, gSens, gKick, gPower;
vec3 gMeanC;

float bandLevel(float w){
    float lvl = pow(clamp(spec(pow(clamp(w, 0.0, 1.0), 1.25) * 0.85) * gSens * (1.0 + 3.5 * w), 0.0, 1.5), 0.6);
    return mix(0.5 + 0.12 * sin(w * 3.0 + time * 0.5), lvl, gLvlLive);   // idle: one gentle swell (no twin lines in a narrow wedge)
}
// fan angle -> wavelength (phi decreases monotonically with w); returns -1 outside
float wForPhi(float phi){
    for (int k = 0; k < K - 1; k++){
        float a = gPhi[k], b = gPhi[k + 1];
        if (phi <= a && phi >= b) return (float(k) + (a - phi) / max(a - b, 1e-5)) / float(K - 1);
    }
    return phi > gPhi[0] ? -(phi - gPhi[0]) : -1.0 - (gPhi[K - 1] - phi);
}
// colour of the fan at wavelength w, averaged over a window (overlap -> white), and level
vec3 colorWindow(float w, float sigma){
    vec3 c = vec3(0.0);
    for (int j = -3; j <= 3; j++){
        float ww = w + sigma * float(j) / 3.0;
        c += (ww < 0.0 || ww > 1.0) ? gMeanC : wavelengthColor(ww);
    }
    return c / 7.0 / gMeanC * WHITE;
}
vec3 fanColor(float w, float sigma, float lvlBlur, out float lvl){
    // colour is averaged over the overlap window (-> white near the exit); the band level is
    // taken at the centre wavelength so absorption lines stay crisp and don't ripple
    vec3 c = vec3(0.0);
    for (int j = -3; j <= 3; j++){
        float ww = w + sigma * float(j) / 3.0;
        c += (ww < 0.0 || ww > 1.0) ? gMeanC : wavelengthColor(ww);
    }
    lvl = lvlBlur > 0.0
        ? (bandLevel(w - lvlBlur) + 2.0 * bandLevel(w) + bandLevel(w + lvlBlur)) * 0.25
        : bandLevel(w);
    return c / 7.0 / gMeanC * WHITE * (0.06 + 2.4 * lvl * lvl);
}

// light from the fan at point p (soft = extra edge softness for the blurred reflection)
vec3 fanAt(vec2 p, float soft, float hazeN, out float fanMask){
    fanMask = 0.0;
    if (!gOk) return vec3(0.0);
    vec2 d = p - gE;
    float r = length(d);
    if (r < 0.004 || d.x < -0.05) return vec3(0.0);
    float phi = atan(d.y, d.x);
    float span = max(gPhi[0] - gPhi[K - 1], 1e-3);
    float W = 0.014 + soft;                               // the beam has width -> soft wedge edges
    float s = max(W / (r + 0.02), 0.6 * min(span, 0.04));   // a narrow wedge stays one filled beam, not two hairlines
    float m = smoothstep(gPhi[K - 1] - s, gPhi[K - 1] + s, phi) * smoothstep(gPhi[0] + s, gPhi[0] - s, phi);
    if (m <= 0.0) return vec3(0.0);
    float w = clamp((gPhi[0] - phi) / span, 0.0, 1.0);
    float wi = wForPhi(clamp(phi, gPhi[K - 1], gPhi[0]));
    if (wi >= 0.0) w = wi;
    float Wc = 0.10 + 0.04 * gKick + soft;                // beam aperture: near the exit the colours overlap -> white
    float sigma = clamp(Wc / (r * span + 1e-4), 0.0, 1.0);
    float lvl;
    vec3 c = fanColor(w, sigma, 0.08, lvl) * 0.5;       // softened lines in the smoke
    // termination: card face or floor
    vec2 dir = d / r;
    float tEnd = 6.0;
    if (dir.x > 1e-3){
        float tc = (CARD_X - gE.x) / dir.x;
        float yc = gE.y + dir.y * tc;
        if (yc > FLOOR_Y && yc < FLOOR_Y + (CARD_TOP - FLOOR_Y) + 0.0) tEnd = min(tEnd, tc);
    }
    if (dir.y < -1e-3) tEnd = min(tEnd, (FLOOR_Y - gE.y) / dir.y);
    float lit = smoothstep(tEnd + 0.01, tEnd - 0.02, r);
    fanMask = m * lit;
    // light revealed by smoke: mostly haze scatter, a little direct streak texture
    float streak = 0.75 + 0.25 * dot_noise(vec3(phi * 25.0, r * 1.5 - time * 0.2, time * 0.1));
    float energy = gPower / (1.0 + r * 1.6);
    return c * fanMask * energy * (0.15 * streak + 0.09 * 10.0 * hazeN * haze) ;
}

// the matte card catches the fan as a projected spectrum: the spectrograph readout
float cardTopAt(float x){ return FLOOR_Y + (CARD_TOP - FLOOR_Y) * mix(1.0, 0.86, clamp((x - CARD_X) / CARD_W, 0.0, 1.0)); }
float sdCard(vec2 p){
    float sx = max(CARD_X - p.x, p.x - (CARD_X + CARD_W));
    float sy = max(FLOOR_Y - p.y, p.y - cardTopAt(p.x));
    return max(sx, sy);
}
// the projected spectrum at height y on the card: continuous, with narrow dark lines at quiet bands
vec3 cardSpectrum(float y, out float m){
    m = 0.0;
    if (!gOk) return vec3(0.0);
    float phi = atan(y - gE.y, CARD_X - gE.x);
    float span = max(gPhi[0] - gPhi[K - 1], 1e-3);
    float sEdge = 0.010 / max(CARD_X - gE.x, 0.1);
    m = smoothstep(gPhi[K - 1] - sEdge, gPhi[K - 1] + sEdge, phi) * smoothstep(gPhi[0] + sEdge, gPhi[0] - sEdge, phi);
    float w = wForPhi(clamp(phi, gPhi[K - 1], gPhi[0]));
    if (w < 0.0) w = clamp((gPhi[0] - phi) / span, 0.0, 1.0);
    float lvl = bandLevel(w);
    float base = 0.35 + 0.9 * lvl;
    float lineW = 0.012;
    // irregularly spaced lines (warped grid), cut only where the band is quiet
    float wl = w * 14.0 + 0.45 * sin(w * 23.0 + 1.3) + 0.25 * sin(w * 57.0);
    // notch only RELATIVE dips (a band well below its neighbours) -> a few meaningful lines, not a zebra
    float lvlN = (bandLevel(w - 0.05) + bandLevel(w + 0.05)) * 0.5;
    float notch = smoothstep(0.55, 0.25, lvl / max(lvlN, 0.05)) * smoothstep(lineW, 0.0, abs(fract(wl) - 0.5) * 2.0 / 14.0);
    return colorWindow(w, 0.02) * base * (1.0 - 0.85 * notch) * m * gPower;
}
vec3 cardAt(vec2 p, out float onCard){
    float sd = sdCard(p);
    onCard = smoothstep(0.003, -0.003, sd);
    float t = clamp((p.x - CARD_X) / CARD_W, 0.0, 1.0);
    vec3 c = vec3(0.075, 0.072, 0.07) * (0.75 + 0.25 * smoothstep(FLOOR_Y, CARD_TOP, p.y)) + gFanAvg * 0.04 * gPower;
    float m;
    vec3 fc = cardSpectrum(p.y, m);
    c += fc * 0.32 * (1.0 - 0.35 * t);                      // card angled away: right side dimmer
    float grain = 0.95 + 0.05 * hash12(floor(p * 900.0));
    c *= grain;
    // lit thickness edge on the left (~4 px), catching the spectrum
    float thick = smoothstep(CARD_X - 0.007, CARD_X - 0.004, p.x) * smoothstep(CARD_X + 0.001, CARD_X - 0.001, p.x) * step(p.y, cardTopAt(CARD_X)) * step(FLOOR_Y, p.y);
    onCard = max(onCard, thick);
    c = mix(c, vec3(0.07, 0.07, 0.075) + fc * 0.5, thick);
    return c * onCard;
}
void main(){
    vec2 res = dimensions;
    vec2 pc = vec2(coord.x, res.y - coord.y);
    vec2 uv0 = (2.0 * pc - res) / res.y;
    float aspect = res.x / res.y;
    float dolly = 1.0 + 0.06 * (0.5 + 0.5 * sin(time * 0.021));
    vec2 uv = uv0 / dolly + vec2(0.06 * sin(time * 0.017), 0.02 * sin(time * 0.023));

    float e = totalEnergy();
    float live = smoothstep(0.05, 0.4, e);
    gLvlLive = live;
    gSens = audioSensitivity;
    // relative kick: bass above its own 4 s average, so brickwalled masters don't pin it
    float kick = smoothstep(0.04, 0.22, (band(0.0, 0.08) - bassAvg) * gSens) * live;
    gKick = kick;
    float high = mix(0.15, pow(clamp(band(0.5, 1.0) * gSens * 3.0, 0.0, 1.5), 0.6), live);
    float rms = mix(0.35, clamp(e / 32.0 * 2.2 * gSens, 0.0, 1.2), live);

    // epochs: swing (beam re-aims through minimum deviation) or eclipse
    float ep = time / 12.0;
    float eid = floor(ep), ef = fract(ep);
    float kind = hash12(vec2(eid * 17.13 + 0.5, 9.0));
    float env = sin(3.14159 * clamp((ef - 0.30) / 0.35, 0.0, 1.0));
    float swing = (kind > 0.30 && kind < 0.65) ? env : 0.0;
    float eclipse = kind >= 0.80 ? env * (1.0 - 0.5 * kick) : 0.0;      // rarer: an event, not a habit
    eclipse = max(eclipse, 1.0 - live);                                 // silence rests in the eclipse look

    prismSetup(vec2(-0.55, PED_TOP + 0.5 * 0.36), 0.36);
    float aimF = 0.40 + 0.04 * sin(time * 0.2) + 0.38 * swing * sin(6.2831853 * clamp((ef - 0.30) / 0.35, 0.0, 1.0) + 1.0);
    aimF = clamp(aimF, AIM_LO, AIM_HI);
    gLamp = vec2(-aspect - 0.2, FLOOR_Y + 0.04);            // the lamp is off-frame: the beam comes out of darkness
    vec2 aim = mix(V[2], V[0], aimF);
    vec2 d0 = normalize(aim - gLamp);
    gD0 = d0;
    float disp = dispersion * (1.0 - 0.92 * eclipse);
    gPower = beamPower * (0.55 + 0.6 * rms + 0.35 * kick) * (1.0 - 0.45 * eclipse);
    gPower *= mix(1.8, 1.0, live);                       // silence: a dim but clearly lit eclipse

    // mean colour of the sampled spectrum, so the overlapped fan integrates to white
    gMeanC = vec3(0.0);
    for (int j = 0; j < 12; j++) gMeanC += wavelengthColor((float(j) + 0.5) / 12.0);
    gMeanC /= 12.0;

    // trace K rays through the glass
    vec2 n1; int e1;
    float t1 = hitPrism(gLamp, d0, -1, n1, e1);
    gP1 = gLamp + d0 * min(t1, 8.0);
    gOk = t1 < 1e8;
    vec2 eSum = vec2(0.0); float eN = 0.0;
    float minMargin = 1.0;
    for (int k = 0; k < K; k++){
        float w = float(k) / float(K - 1);
        float n = mix(1.55, mix(1.44 - 0.048 * kick, 1.69 + 0.08 * kick, w), disp);
        vec2 d1 = refract(d0, n1, 1.0 / n);
        vec2 q = gP1 + d1 * 1e-3;
        vec2 n2; int e2;
        float t2 = hitPrism(q, d1, e1, n2, e2);
        vec2 P2 = q + d1 * min(t2, 2.0);
        float cosI = dot(d1, n2);
        minMargin = min(minMargin, 1.0 - n * n * (1.0 - cosI * cosI));   // < 0 = total internal reflection
        vec2 d2 = refract(d1, -n2, n);
        if (k == K / 2){ gD1m = d1; gN2m = n2; gP2m = P2; gE2m = e2; }
        if (t2 > 1e8 || dot(d2, d2) < 0.5){ gOk = false; d2 = vec2(1.0, 0.0); }
        gP2[k] = P2;
        gPhi[k] = atan(d2.y, d2.x);
        eSum += P2; eN += 1.0;
    }
    gE = eSum / eN;
    gPower *= smoothstep(-0.02, 0.06, minMargin);           // fade before TIR rather than popping
    // keep phi strictly decreasing (guards the lookup)
    for (int k = 1; k < K; k++) gPhi[k] = min(gPhi[k], gPhi[k - 1] - 1e-4);

    float hazeN = 0.45 + 0.55 * (0.5 + 0.5 * dot_fbm(vec3(uv * 1.6 + vec2(time * 0.03, 0.0), time * 0.05)));

    // ---------------- backdrop ----------------
    // irregular, half-contrast fluting; dim warm key from the upper left
    float ribPhase = uv.x * 34.0 + 0.6 * sin(uv.x * 5.0);
    float sdG = sdTri(uv);
    if (sdG < 0.0) ribPhase = (uv.x + 0.08 * sdG * 6.0) * 34.0 + 0.6 * sin(uv.x * 5.0) + 1.2;   // refracted through the glass
    float rib = 0.5 + 0.5 * cos(ribPhase);
    float ribShade = mix(0.94, 1.06, rib) * (0.94 + 0.06 * dot_noise(vec3(uv * vec2(2.5, 0.5), 1.0)));
    vec3 wall = mix(vec3(0.010, 0.011, 0.016), vec3(0.020, 0.016, 0.014), smoothstep(-1.5, 1.5, uv.x));
    float keyG = exp(-length((uv - vec2(-1.2, 1.0)) * vec2(0.55, 0.8)) * 1.3);
    wall += vec3(0.10, 0.075, 0.05) * keyG * 1.0 * haze;
    wall += vec3(0.09, 0.07, 0.05) * keyG * hazeN * 0.35 * haze;             // the key lights the smoke
    vec2 ks = uv - vec2(-2.3, 1.7);
    float kAng = atan(ks.y, ks.x);
    float shafts = 0.5 + 0.5 * dot_noise(vec3(kAng * 18.0, time * 0.05, 3.0));
    shafts = pow(shafts, 3.0) * smoothstep(-0.25, -0.85, kAng) * exp(-length(ks) * 0.6);
    wall += vec3(0.16, 0.12, 0.08) * shafts * hazeN * 0.30 * haze;
    // the room breathes with the fan's light (bounce)
    float lvlAvg;
    vec3 fanAvg = fanColor(0.5, 1.0, 0.3, lvlAvg);
    gFanAvg = fanAvg;
    wall += mix(fanAvg, vec3(dot(fanAvg, vec3(0.33))), 0.5) * 0.035 * exp(-length((uv - vec2(0.7, -0.25)) * vec2(0.55, 1.1)) * 1.3) * gPower * (0.8 + 0.6 * kick) * haze;
    wall += mix(fanAvg, vec3(0.3), 0.5) * 0.030 * exp(-length((uv - vec2(0.4, 0.15)) * vec2(0.5, 0.9)) * 1.1) * gPower * haze;
    wall *= ribShade;

    vec3 col = wall;

    // ---------------- lamp ----------------
    vec2 lampDir = d0;
    col += vec3(0.10, 0.09, 0.08) * exp(-length((uv - gLamp) * vec2(0.6, 1.4)) * 2.2) * 0.25 * haze;   // spill from the off-frame lamp

    // ---------------- incoming beam + glass ----------------
    float h;
    float dIn = segDist(uv, gLamp, gP1, h);
    float surge = kick * exp(-pow((h - (1.0 - kick)) * 5.0, 2.0)) * 2.0;
    col += WHITE * (exp(-dIn * dIn / (0.0045 * 0.0045)) * 0.9 + exp(-dIn / 0.05) * 0.07 * hazeN * haze) * gPower * (1.0 + surge);
    // inside the glass: the beam begins to split
    for (int k = 0; k < K; k += 2){
        float hh;
        float di = segDist(uv, gP1, gP2[k], hh);
        col += wavelengthColor(float(k) / float(K - 1)) / gMeanC * WHITE * exp(-di * di / (0.006 * 0.006)) * 0.05 * gPower * mix(1.0, 0.2, hh);
    }

    // pedestal: matte black block with a faint top edge highlight and contact shadow
    float ped = sdBox(uv, vec2(-0.80, FLOOR_Y), vec2(-0.30, PED_TOP));
    float topFace = smoothstep(PED_TOP - 0.032, PED_TOP - 0.028, uv.y) * step(uv.y, PED_TOP) * step(abs(uv.x + 0.55), 0.25);
    float pedM = smoothstep(0.003, -0.003, ped);
    // matte black block: lamp-lit left face gradient, bevelled top edge, faint key on the top
    vec3 pedC = vec3(0.022, 0.022, 0.026) * (0.7 + 0.6 * smoothstep(-0.30, -0.80, uv.x)) * (0.6 + 0.4 * smoothstep(FLOOR_Y, PED_TOP, uv.y));
    pedC += vec3(0.09, 0.09, 0.10) * exp(-abs(uv.y - PED_TOP) * 260.0) * step(abs(uv.x + 0.55), 0.25);
    pedC += vec3(0.05, 0.05, 0.055) * exp(-abs(abs(uv.x + 0.55) - 0.25) * 300.0) * step(uv.y, PED_TOP);
    pedC += fanAvg * 0.02 * smoothstep(-0.30, -0.80, uv.x) * gPower;
    pedC = mix(pedC, vec3(0.06, 0.06, 0.065) + fanAvg * 0.03 * gPower, topFace);   // the lit top face
    col = mix(col, pedC, pedM);

    // the glass body: refracted backdrop, soft rim, bevel, sheen, chromatic fringe, contact caustic
    float inside = smoothstep(0.003, -0.003, sdG);
    vec3 glassC = col * 0.9;
    float sheen = smoothstep(0.07, 0.0, abs(dot(uv - CEN, normalize(vec2(1.0, 1.3))) + 0.05 - 0.04 * sin(time * 0.2))) * 0.06 * glass;
    glassC += vec3(0.8, 0.85, 1.0) * sheen;
    col = mix(col, glassC, inside);
    // rim: only the face the light enters really glows
    float bestD = -1e9, faceW = 0.25;
    for (int k = 0; k < 3; k++){
        float dk = dot(uv - V[k], NRM[k]);
        if (dk > bestD){ bestD = dk; faceW = 0.25 + 0.75 * max(dot(NRM[k], -gD0), 0.0); }
    }
    col += vec3(0.85, 0.9, 1.0) * exp(-abs(sdG) * 180.0) * 0.30 * glass * faceW;
    // faint coloured internal reflection off the exit face, back along the base
    if (gOk){
        vec2 dr = reflect(gD1m, gN2m);
        vec2 nr; int er;
        float tr = hitPrism(gP2m + dr * 1e-3, dr, gE2m, nr, er);
        if (tr < 1e8){
            float hr;
            float drd = segDist(uv, gP2m, gP2m + dr * tr, hr);
            col += mix(fanAvg, WHITE, 0.5) * exp(-drd / 0.03) * 0.010 * (1.0 - hr) * gPower * glass * inside;   // a glow, not a line
        }
    }
    vec3 fringe = vec3(exp(-abs(sdG - 0.003) * 260.0), exp(-abs(sdG) * 260.0), exp(-abs(sdG + 0.003) * 260.0));
    col += fringe * 0.08 * glass;
    col += vec3(1.0, 0.95, 0.9) * exp(-length(uv - V[0]) * 50.0) * 0.25 * glass;
    // caustic pool on the pedestal top, under the glass
    col += (fanAvg * 0.5 + WHITE * 0.5) * exp(-pow((uv.x - gE.x + 0.05) / 0.12, 2.0) - pow((uv.y - PED_TOP + 0.016) / 0.012, 2.0)) * 0.30 * gPower * glass;

    // halation where the fan leaves the glass (tight), streak on kicks
    float hd = length(uv - gE);
    col += WHITE * (exp(-hd * 36.0) * 0.35 + exp(-hd * 8.0) * 0.03) * gPower * (0.7 + 1.0 * kick);

    // ---------------- the fan, the card ----------------
    float fanM;
    vec3 fan = fanAt(uv, 0.0, hazeN, fanM);
    col += fan;
    float onCard;
    vec3 card = cardAt(uv, onCard);
    float haloM;
    vec3 haloC = cardSpectrum(clamp(uv.y, FLOOR_Y, CARD_TOP), haloM);
    col += mix(haloC, vec3(dot(haloC, vec3(0.33))), 0.5) * exp(-max(sdCard(uv), 0.0) / 0.05) * 0.04 * step(uv.x, CARD_X) * (1.0 - onCard) * step(FLOOR_Y, uv.y);
    col = mix(col, card + col * 0.15, onCard);
    // dust motes in the fan, glittering with highs
    vec2 gq = uv * 80.0 + vec2(time * 0.15, time * 0.05);
    vec2 gid = floor(gq);
    float dh = hash12(gid);
    vec2 off = vec2(hash12(gid + 1.7), hash12(gid + 4.3)) - 0.5;
    float mote = smoothstep(0.975, 1.0, dh) * exp(-length(fract(gq) - 0.5 - off * 0.6) * 9.0) * (0.5 + 0.5 * sin(time * 6.0 + dh * 60.0));
    col += WHITE * mote * fanM * (0.1 + 1.2 * high) * 0.8;

    // mild global desaturation: light, not poster
    col = mix(vec3(dot(col, vec3(0.3, 0.59, 0.11))), col, 0.78);

    // ---------------- floor ----------------
    if (uv.y < FLOOR_Y){
        float depthBelow = FLOOR_Y - uv.y;
        vec2 pm = vec2(uv.x, 2.0 * FLOOR_Y - uv.y);
        float rough = clamp(0.65 + 0.35 * dot_noise(vec3(uv * vec2(8.0, 30.0), 2.0)) / 3.0, 0.3, 1.0);   // stays positive (NaN guard)
        float soft = depthBelow * 0.18 * (1.3 - 0.5 * floorGloss) * rough;
        float fm;
        vec3 rFan = fanAt(pm + vec2(0.0, 0.0), soft, hazeN, fm);
        float oc;
        vec3 rCard = cardAt(pm, oc);
        float dInM = segDist(pm, gLamp, gP1, h);
        vec3 rBeam = WHITE * exp(-dInM / (0.006 + soft)) * 0.25 * gPower;
        float rPed = smoothstep(0.003, -0.003, sdBox(pm, vec2(-0.80, FLOOR_Y), vec2(-0.30, PED_TOP)));
        vec3 rScene = (rFan * 0.8 + rCard * 0.7 + rBeam) * (1.0 - rPed * 0.9);
        float fres = mix(0.45, 0.06, smoothstep(0.0, 0.3, depthBelow)) * floorGloss;
        // pools where the fan reaches the floor, spread in perspective
        vec3 pool = vec3(0.0);
        if (gOk){
            float phiF = atan(FLOOR_Y - gE.y, uv.x - gE.x);
            float span = max(gPhi[0] - gPhi[K - 1], 1e-3);
            float sF = 0.02;
            float mF = smoothstep(gPhi[K - 1] - sF, gPhi[K - 1] + sF, phiF) * smoothstep(gPhi[0] + sF, gPhi[0] - sF, phiF);
            float fadeX = smoothstep(CARD_X + 0.02, CARD_X - 0.25, uv.x) * smoothstep(gE.x, gE.x + 0.5, uv.x);
            float wF = wForPhi(clamp(phiF, gPhi[K - 1], gPhi[0]));
            if (wF < 0.0) wF = 0.5;
            float lv;
            vec3 pc2 = fanColor(wF, 0.05, 0.04, lv);
            float sy = 0.004 + 0.08 * depthBelow;
            pool = mix(pc2, vec3(dot(pc2, vec3(0.33))), 0.4) * mF * exp(-depthBelow / sy * 0.5) * smoothstep(0.0, 0.012, depthBelow) * fadeX * gPower * 0.22 * (1.0 + 0.6 * kick);
            // light at the foot of the card
            pool += fanAvg * gPower * 0.06 * exp(-pow((uv.x - CARD_X - 0.1) / 0.25, 2.0)) * exp(-depthBelow * 12.0);
        }
        vec3 floorC = vec3(0.010, 0.010, 0.013) + wall * 0.5 * exp(-depthBelow * 6.0);   // soft wall-to-floor bounce
        col = floorC + rScene * fres + pool;
        // contact shadows under the pedestal and card
        col *= 1.0 - 0.6 * exp(-depthBelow * 25.0) * (smoothstep(0.03, -0.02, abs(uv.x + 0.55) - 0.25) + smoothstep(0.03, -0.02, abs(uv.x - CARD_X - CARD_W * 0.5) - CARD_W * 0.5));
    } else {
        col += wall * 0.4 * exp(-(uv.y - FLOOR_Y) * 10.0);   // wall meets floor softly
        // the floor pool grazes up onto the wall so the seam isn't ruler-straight
        if (gOk && uv.y < FLOOR_Y + 0.03){
            float phiF = atan(FLOOR_Y - gE.y, uv.x - gE.x);
            float sF = 0.02;
            float mF = smoothstep(gPhi[K - 1] - sF, gPhi[K - 1] + sF, phiF) * smoothstep(gPhi[0] + sF, gPhi[0] - sF, phiF);
            float fadeX = smoothstep(CARD_X + 0.02, CARD_X - 0.25, uv.x) * smoothstep(gE.x, gE.x + 0.5, uv.x);
            float wF = wForPhi(clamp(phiF, gPhi[K - 1], gPhi[0]));
            float lv;
            vec3 pc2 = fanColor(max(wF, 0.0), 0.05, 0.04, lv);
            col += mix(pc2, vec3(dot(pc2, vec3(0.33))), 0.4) * mF * fadeX * gPower * 0.22 * 0.3 * exp(-(uv.y - FLOOR_Y) / 0.012);
        }
    }

    float vig = smoothstep(2.1, 0.5, length(uv0 * vec2(0.72, 1.0)));
    col *= 0.6 + 0.4 * vig;
    col = tonemap(hueRotate(col, hueShift * 6.2831853), brightness);
    col *= 0.85 + 0.15 * live;
    // temporal smoothing on the card only (frame feedback; raw y-down texture space)
    float fs = clamp(frameDt * 30.0, 0.25, 3.0);
    vec3 prevC = texture(tex, coord / res).rgb;
    col = mix(col, prevC, pow(0.65, fs) * onCard);
    outputColor = vec4(col, 1.0);
}
