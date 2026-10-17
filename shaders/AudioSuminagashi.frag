#version 150

uniform sampler2D tex;
uniform vec2 dimensions;
uniform float time;
uniform float audio[256];
uniform float frameDt;
uniform float bassAvg;
uniform float midAvg;
uniform float highAvg;
uniform float beatPulse;
uniform float bpm;
uniform float dropForce;
uniform float kickPunch;
uniform float current;
uniform float gestureStrength;
uniform float shimmer;
uniform float separation;
uniform float tension;
uniform float vignette;
uniform float audioSensitivity;

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

//U frameDt = 0.0333
//U bassAvg = 0.0
//U midAvg = 0.0
//U highAvg = 0.0
//U beatPulse = 0.0
//U bpm = 120.0
//U dropForce = 1.0
//U kickPunch = 1.0
//U current = 1.0
//U gestureStrength = 1.0
//U shimmer = 1.0
//U separation = 1.0
//U tension = 1.0
//U vignette = 1.0
//U audioSensitivity = 1.0
// Audio Suminagashi — Japanese ink marbling on lit water, driven by the music.
// The output IS the state (frame feedback): every frame the previous surface is advected
// by the water's velocity (Catmull-Rom, letting bilinear softness win on hairlines), new
// ink / clear water is dropped in, and blurred tints are pulled back toward pure ink
// along their own density (inks are immiscible films). Kicks drop ink at the bass site
// and clear water between kicks, pushing earlier ink outward into rings, and give the
// whole field a visible jolt; mids feed a second site; hi-hats shimmer the surface. On a
// slow epoch clock a marbling gesture sweeps through: comb, wave comb, stylus swirl or a
// stone drop. A reversible display transform (vignette, washi grain, slow caustic sheen)
// is applied on output and divided back out when the state is read.

const float GOLDEN = 2.39996323;

vec3 desat(vec3 c, float k){ return mix(vec3(dot(c, vec3(0.3, 0.59, 0.11))), c, 1.0 - k); }

vec3 paletteInk(int p, int i){
    vec3 c;
    if (p == 0){        // Sumi & rust: indigo, rust, ochre, ivory
        c = i == 0 ? vec3(0.20, 0.30, 0.56) : i == 1 ? vec3(0.72, 0.30, 0.15) : i == 2 ? vec3(0.84, 0.64, 0.30) : vec3(0.86, 0.82, 0.72);
    } else if (p == 1){ // Celadon: deep jade (dominant), celadon, plum, pearl
        c = i == 0 ? vec3(0.12, 0.40, 0.36) : i == 1 ? vec3(0.50, 0.68, 0.58) : i == 2 ? vec3(0.50, 0.20, 0.36) : vec3(0.86, 0.86, 0.80);
    } else {            // Ember: oxblood, copper, saffron, bone
        c = i == 0 ? vec3(0.60, 0.09, 0.11) : i == 1 ? vec3(0.86, 0.46, 0.20) : i == 2 ? vec3(0.98, 0.76, 0.28) : vec3(0.88, 0.84, 0.76);
    }
    return desat(c, (p == 0 && i == 0) ? 0.3 : 0.15);   // the dominant indigo is quieter
}
// Weighted pick: one ink dominates, the light ink (index 3) is rare.
int pickInk(float h){ return h < 0.55 ? 0 : (h < 0.85 ? 1 : (h < 0.97 ? 2 : 3)); }

vec3 waterAt(vec2 uv, float t, int pal){
    float n = dot_noise(vec3(uv * 0.9, t * 0.02)) / 3.0;
    vec3 w = mix(vec3(0.090, 0.105, 0.120), vec3(0.130, 0.150, 0.170), 0.5 + 0.5 * n);
    return mix(w, paletteInk(pal, 0) * 0.2, 0.2);
}

// A finished marbling to start from, so the surface is full from the first frame.
vec3 seedPattern(vec2 uv, int p){
    vec2 w = uv + 0.28 * vec2(dot_fbm(vec3(uv * 1.2, 1.0)), dot_fbm(vec3(uv * 1.2, 7.0)));
    vec3 water = waterAt(uv, 0.0, p);
    vec3 c = water;
    for (int k = 0; k < 6; k++){
        vec2 ck = vec2((hash12(vec2(float(k), 1.3)) - 0.5) * 3.2, (hash12(vec2(float(k), 5.1)) - 0.5) * 1.8);
        float R = 0.35 + 0.45 * hash12(vec2(float(k), 9.7));
        float d = length(w - ck);
        if (d < R){
            float sp = 0.05 + 0.04 * hash12(vec2(float(k), 3.3));
            float ph = d / sp + 0.9 * sin(d * 9.0 + float(k)) + 0.5 * dot_noise(vec3(w * 4.0, float(k)));
            float ring = floor(ph);
            float f = fract(ph);
            float edge = smoothstep(0.0, 0.12, f) * smoothstep(1.0, 0.88, f);
            vec3 ink = paletteInk(p, pickInk(hash12(vec2(ring, float(k) + 0.5))));
            float gran = 0.8 + 0.2 * dot_noise(vec3(uv * 9.0, 2.0)) / 3.0 + 0.08 * dot_noise(vec3(uv * 35.0, 4.0)) / 3.0;
            c = mod(ring, 2.0) < 0.5 ? mix(c, ink * gran, edge * 0.85) : mix(c, water, edge);
        }
    }
    return c;
}

// Display transform: vignette x washi grain x slow caustic sheen. Applied on output and
// divided back out (at the sample's source position and previous time) when the state
// is read. Kept within ~0.80..1.05 so 8-bit quantisation stays benign.
float displayL(vec2 u, float aspect, float t){
    float vg = smoothstep(1.9, 0.3, length(u * vec2(1.0 / aspect * 1.6, 1.0)));
    float fiber = dot_noise(vec3(u.x * 3.0, u.y * 38.0, 2.0)) / 3.0;
    float caustic = dot_noise(vec3(u * 2.4, t * 0.15)) / 3.0;
    float L = (0.82 + 0.18 * vg) * (0.975 + 0.025 * fiber) * (1.0 + 0.07 * caustic);
    return clamp(mix(1.0, L, vignette), 0.78, 1.08);
}

// Catmull-Rom resample (9 bilinear taps) clamped to the 2x2 neighbourhood; also returns
// the local contrast of that neighbourhood.
vec3 sampleCR(vec2 uvT, vec2 texSize, out float grad){
    vec2 sp = uvT * texSize;
    vec2 t1 = floor(sp - 0.5) + 0.5;
    vec2 f = sp - t1;
    vec2 w0 = f * (-0.5 + f * (1.0 - 0.5 * f));
    vec2 w1 = 1.0 + f * f * (-2.5 + 1.5 * f);
    vec2 w2 = f * (0.5 + f * (2.0 - 1.5 * f));
    vec2 w3 = f * f * (-0.5 + 0.5 * f);
    vec2 w12 = w1 + w2;
    vec2 t12 = (t1 + w2 / w12) / texSize;
    vec2 t0 = (t1 - 1.0) / texSize;
    vec2 t3 = (t1 + 2.0) / texSize;
    vec3 r = texture(tex, vec2(t0.x, t0.y)).rgb * w0.x * w0.y
           + texture(tex, vec2(t12.x, t0.y)).rgb * w12.x * w0.y
           + texture(tex, vec2(t3.x, t0.y)).rgb * w3.x * w0.y
           + texture(tex, vec2(t0.x, t12.y)).rgb * w0.x * w12.y
           + texture(tex, vec2(t12.x, t12.y)).rgb * w12.x * w12.y
           + texture(tex, vec2(t3.x, t12.y)).rgb * w3.x * w12.y
           + texture(tex, vec2(t0.x, t3.y)).rgb * w0.x * w3.y
           + texture(tex, vec2(t12.x, t3.y)).rgb * w12.x * w3.y
           + texture(tex, vec2(t3.x, t3.y)).rgb * w3.x * w3.y;
    vec2 b = t1 / texSize, px = 1.0 / texSize;
    vec3 a0 = texture(tex, b).rgb, a1 = texture(tex, b + vec2(px.x, 0.0)).rgb;
    vec3 a2 = texture(tex, b + vec2(0.0, px.y)).rgb, a3 = texture(tex, b + px).rgb;
    vec3 mn = min(min(a0, a1), min(a2, a3)), mx = max(max(a0, a1), max(a2, a3));
    grad = length(mx - mn);
    // where the clamp actually clips, channels clip unevenly (chroma fringes): lean on the
    // bilinear sample there instead
    vec3 rc = clamp(r, mn, mx);
    return mix(rc, texture(tex, uvT).rgb, smoothstep(0.0, 0.04, length(r - rc)));
}

vec2 toTex(vec2 u, vec2 res){ return vec2((u.x * res.y / res.x) * 0.5 + 0.5, 0.5 - u.y * 0.5); }

vec2 bassSiteFor(float id, float aspect){
    float s = id * 17.13 + 0.5;
    return vec2((hash12(vec2(s, 1.0)) - 0.5) * aspect * 1.2, (hash12(vec2(s, 2.0)) - 0.5) * 1.1);
}
vec2 midSiteFor(float id, float aspect){
    // golden-angle offset from the bass site, kept on screen: never side-by-side twins
    vec2 b = bassSiteFor(id, aspect);
    float a = id * GOLDEN + 1.0;
    vec2 m = b + 0.85 * vec2(cos(a), sin(a));
    m.x = clamp(m.x, -aspect * 0.75, aspect * 0.75);
    m.y = clamp(m.y, -0.75, 0.75);
    if (abs(m.y - b.y) < 0.4) m.y = b.y + (b.y > 0.0 ? -0.5 : 0.5);
    return m;
}

// Drop ink / clear water at a site: ragged edge, streaky internal pigment, deeper rim.
vec3 dropInto(vec3 col, vec2 uv, vec2 d, float r0, float S, vec3 ink, vec3 water, float rate){
    // S here is the INK commitment (0 water .. 1 ink); callers derive it from beat-phase
    // windows so it changes once per window, never frame-to-frame
    float dd = length(d) + 0.006 * dot_noise(vec3(uv * 22.0, time));
    float m = smoothstep(r0, r0 * 0.88, dd);
    float rim = smoothstep(r0 * 0.80, r0 * 0.97, dd) * m;
    float gran = 0.8 + 0.2 * dot_noise(vec3(uv * 9.0, time * 0.1)) / 3.0 + 0.08 * dot_noise(vec3(uv * 35.0, time * 0.1)) / 3.0;
    vec3 inkC = mix(ink * gran, ink * ink * 0.85, rim * 0.9);
    vec3 dropC = mix(water, inkC, smoothstep(0.35, 0.45, S));          // committed: ink or water
    // sites always inject: ink on the onset, clear water the rest of the time -> rings
    return mix(col, dropC, m * rate);
}

void main(){
    vec2 res = dimensions;
    vec2 pc = vec2(coord.x, res.y - coord.y);
    vec2 uv = (2.0 * pc - res) / res.y;
    float aspect = res.x / res.y;

    float e = totalEnergy();
    float live = smoothstep(0.05, 0.4, e);
    float sens = audioSensitivity;
    float fs = clamp(frameDt * 30.0, 0.25, 3.0);
    float dt = fs / 30.0;

    float idle = smoothstep(0.55, 1.0, sin(time * 1.7));
    // relative onsets against engine-side 4 s EMAs (bassAvg/midAvg/highAvg), so brickwalled
    // masters (sub pinned high) still get ink on the kick and clear water between kicks
    float bassK = mix(idle, smoothstep(0.03, 0.13, (band(0.0, 0.08) - bassAvg) * sens), live);
    float midK  = mix(0.4 * idle, smoothstep(0.02, 0.10, (band(0.10, 0.45) - midAvg) * sens), live);
    float hiK   = mix(0.05, smoothstep(0.01, 0.08, (band(0.50, 1.00) - highAvg) * sens), live);
    float num = 0.0, den = 1e-4;
    for (int i = 0; i < 32; i++){ float a = audio[i * 8]; num += a * float(i); den += a; }
    float timbre = clamp(num / den / 16.0, 0.0, 1.0);

    float ep = time / 12.0;
    float eid = floor(ep);
    float ef = fract(ep);
    float es = eid * 17.13 + 0.5;
    int pal = int(mod(floor(time / 36.0), 3.0));
    int gesture = hash12(vec2(es, 11.0)) < 0.6 ? int(floor(hash12(vec2(es, 4.0)) * 4.0)) : -1;

    // sites glide from last epoch's position to this one's, leaving a wake
    float glide = smoothstep(0.0, 0.2, ef);
    vec2 bassSite = mix(bassSiteFor(eid - 1.0, aspect), bassSiteFor(eid, aspect), glide) + 0.12 * vec2(sin(time * 0.21), cos(time * 0.17));
    vec2 midSite  = mix(midSiteFor(eid - 1.0, aspect), midSiteFor(eid, aspect), glide) + 0.10 * vec2(cos(time * 0.13), sin(time * 0.19));
    float bpmE = bpm > 40.0 ? bpm : 120.0;
    float beatSlot = floor(time * bpmE / 60.0);         // engine tempo, not a fixed clock
    bassSite += 0.025 * (vec2(hash12(vec2(beatSlot, 1.1)), hash12(vec2(beatSlot, 2.3))) - 0.5);
    midSite  += 0.030 * (vec2(hash12(vec2(beatSlot, 3.1)), hash12(vec2(beatSlot, 4.3))) - 0.5);

    vec2 hiSite = -bassSite * 0.8 + 0.10 * vec2(sin(time * 0.15 + 2.0), cos(time * 0.12))
                + 0.04 * vec2(cos(beatSlot * GOLDEN), sin(beatSlot * GOLDEN));

    // ---- velocity ----
    vec2 v = vec2(0.0);
    vec2 db = uv - bassSite;  float rb = length(db) + 0.03;
    vec2 dm = uv - midSite;   float rm = length(dm) + 0.03;
    vec2 dh = uv - hiSite;    float rh = length(dh) + 0.03;
    v += db / rb * (0.085 * (0.08 + bassK) / rb) * exp(-rb * 0.6) * dropForce;
    v += dm / rm * (0.050 * (0.08 + midK) / rm) * exp(-rm * 0.6) * dropForce;
    v += dh / rh * (0.028 * (0.08 + hiK) / rh) * exp(-rh * 0.8) * dropForce;
    // the kick: a wide-reach impulse so the whole ring field visibly jumps
    v += db / rb * (0.25 * pow(bassK, 3.0) / rb) * exp(-rb * mix(0.6, 0.25, bassK)) * kickPunch;

    float ex = 0.02;
    vec3 np = vec3(uv * 1.1, time * 0.04);
    float n0 = dot_noise(np), nx = dot_noise(np + vec3(ex, 0.0, 0.0)), ny = dot_noise(np + vec3(0.0, ex, 0.0));
    v += vec2(ny - n0, -(nx - n0)) / ex * 0.011 * current;
    // breath: the artist blows gently across the bath -- a slow low-frequency shear that
    // skews the rings into flowing egg and comma shapes instead of op-art targets
    v += vec2(0.07 * sin(uv.y * 1.6 + time * 0.11), 0.055 * sin(uv.x * 1.3 - time * 0.09 + 1.0)) * current;
    // a slow vortex at the bass site turns the rings into spirals
    v += vec2(-db.y, db.x) / rb * (0.02 / rb) * exp(-rb * 0.8) * current;

    float gw = smoothstep(0.45, 0.5, ef) * (1.0 - smoothstep(0.7, 0.75, ef)) * gestureStrength;
    float gp = clamp((ef - 0.45) / 0.3, 0.0, 1.0);
    vec2 stoneC = vec2((hash12(vec2(es, 8.0)) - 0.5) * aspect, (hash12(vec2(es, 9.0)) - 0.5));
    if (gesture == 0 || gesture == 1){
        // comb: a row of tines drawn across (gesture 1 waves them -> chevrons)
        float dir = hash12(vec2(es, 6.0)) > 0.5 ? 1.0 : -1.0;
        float spacing = 0.16;
        float front = dir * (gp * 2.0 - 1.0) * aspect * 1.25;
        float speed = 0.7 * 2.0 * aspect * 1.25 / (0.3 * 12.0);
        float y = uv.y, wob = 0.0;
        if (gesture == 1){ wob = 0.07 * sin(front * 7.0); y -= wob; }
        float ty = (floor(y / spacing) + 0.5) * spacing;
        vec2 dd = uv - vec2(front, ty + wob);
        float pull = exp(-dot(dd, dd) / (0.06 * 0.06)) + 0.35 * exp(-dd.y * dd.y / (0.08 * 0.08)) * exp(-dd.x * dd.x / (0.25 * 0.25));
        v.x += dir * speed * pull * gw * smoothstep(aspect, aspect * 0.85, abs(uv.x));
    } else if (gesture == 2){
        // stylus swirl: a point traces a spiral, dragging the surface with it
        float a = gp * 12.0;
        float rr = 0.15 + 0.5 * gp;
        vec2 sp = stoneC + rr * vec2(cos(a), sin(a));
        vec2 sv = 0.67 * vec2(-sin(a), cos(a));
        v += sv * exp(-dot(uv - sp, uv - sp) / (0.05 * 0.05)) * 3.0 * gw;
    } else if (gesture == 3){
        // stone: a big source opens a new colour field, pushing everything aside
        vec2 d = uv - stoneC;
        float r = length(d) + 0.03;
        v += d / r * (0.11 / r) * exp(-r * 0.5) * gw;
    }

    // ---- advect (step clamped near sources) + hi-hat shimmer ----
    vec2 adv = v * dt;
    float sl = length(adv);
    if (sl > 0.04) adv *= 0.04 / sl;
    vec2 back = uv - adv;
    back += 0.004 * hiK * shimmer * vec2(dot_noise(vec3(uv * 40.0, time * 6.0)), dot_noise(vec3(uv * 40.0 + 5.0, time * 6.0))) / 3.0;
    vec2 bt = toTex(back, res);
    vec2 px = 1.0 / res;
    float edgeF = smoothstep(-0.02, 0.0, min(min(bt.x, 1.0 - bt.x), min(bt.y, 1.0 - bt.y)));   // fade only when sampling outside
    bt = clamp(bt, px * 0.5, 1.0 - px * 0.5);

    float Lprev = displayL(back, aspect, time - frameDt);
    float grad;
    vec3 c0 = texture(tex, bt).rgb;
    vec3 col = sampleCR(bt, res, grad) / Lprev;
    // hairline regions: let bilinear softness win so sub-pixel rings don't crawl
    col = mix(col, c0 / Lprev, 0.5 * smoothstep(0.6, 1.0, grad));

    vec3 water = waterAt(uv, time, pal);
    if (dot(c0, c0) < 1e-6) col = seedPattern(uv, pal);
    col = mix(water, col, edgeF);

    float rate = clamp(dt * 24.0, 0.0, 1.0);
    float hb = mix(hash12(vec2(beatSlot, 7.7)), timbre, 0.5);

    // ---- drops: ink on the beat, clear water between -> rings ----
    float bp = mod(beatSlot, 8.0);
    float barScale = bp < 0.5 ? 1.35 : (mod(bp, 2.0) < 0.5 ? 1.0 : 0.6);  // bar phase unknown: keep the downbeat modest
    // commit ink with the engine's beat pulse: its smooth decay lays one solid band per beat
    // even when the raw onset flickers frame to frame
    // beat phase from the engine tempo; windows decide ink vs water, onsets only size drops
    float bphase = fract(time * bpmE / 60.0);
    float bassInk = live > 0.5 ? smoothstep(0.30, 0.50, 0.9 * beatPulse) : bassK;
    float midInk  = live > 0.5 ? step(0.5, bphase) * step(bphase, 0.68) * step(0.35, midK) : midK;
    float hiInk   = live > 0.5 ? step(0.5, fract(bphase * 2.0 + 0.5)) * step(fract(bphase * 2.0 + 0.5), 0.62) * step(0.3, hiK) : hiK;
    float bassS = max(bassK, 0.9 * beatPulse * live);
    col = dropInto(col, uv, db, (0.05 + 0.10 * bassS) * barScale, bassInk, paletteInk(pal, pickInk(hb)), water, rate);
    col = dropInto(col, uv, dm, 0.6 * (0.05 + 0.10 * midK), midInk, paletteInk(pal, pickInk(fract(hb + 0.37))), water, rate);
    col = dropInto(col, uv, dh, 0.03 + 0.04 * hiK, hiInk, paletteInk(pal, pickInk(fract(hb + 0.71))), water, rate);
    if (gesture == 3){
        float alt = smoothstep(0.45, 0.55, abs(fract(time * 1.5) - 0.5) * 2.0);
        col = dropInto(col, uv, uv - stoneC, 0.07, gw * (1.0 - alt), paletteInk(pal, pickInk(fract(es * 0.13))), water, rate * step(0.01, gw));
    }

    // ---- separation: pull blurred tints back toward the nearest pure ink, along their
    //      own density (hue only), only where the pixel already sits close to that ink line
    {
        vec3 cw = col - water;
        float cl = length(cw);
        vec3 best = water;
        float bestCos = -1.0;
        for (int i = 0; i < 4; i++){
            vec3 iw = paletteInk(pal, i) - water;
            float cs = dot(cw, iw) / (cl * length(iw) + 1e-5);
            if (cs > bestCos){ bestCos = cs; best = paletteInk(pal, i); }
        }
        vec3 bw = best - water;
        float a = clamp(dot(cw, bw) / dot(bw, bw), 0.0, 1.0);
        vec3 tgt = water + bw * a;
        float bd = length(col - tgt);
        float near = 1.0 - smoothstep(0.03, 0.12, bd);
        // hue: drift blurred tints back onto the ink's own line (density kept)
        float dense = smoothstep(0.04, 0.12, cl);       // faint residue has no reliable hue
        col = mix(col, tgt, (1.0 - pow(1.0 - 0.012, fs)) * separation * near * dense * (1.0 - smoothstep(0.03, 0.45, grad)));
        // surface tension: push mid densities (blurred ink/water interfaces) apart so edges
        // stay ~1-2 px soft instead of diffusing; dense ink (0.8..1) keeps its granulation
        // only at interfaces (uniform thin films are left alone) and only when the hue match
        // is confident (grey mixtures of complementary inks must not grow into the pale ink)
        float aT = smoothstep(0.17, 0.67, a) * max(a, 0.8);   // unstable point ~0.43: edges sharpen, don't advance
        float conf = smoothstep(0.80, 0.95, bestCos) * (1.0 - smoothstep(0.5, 0.9, grad));
        float atEdge = smoothstep(0.04, 0.15, grad) * conf;
        // the bass site's divergent flow magnifies resampling blur every frame: boost tension
        // locally there and drop the low-gradient gate (safe: the site rewrites itself)
        float siteW = exp(-rb * rb / (0.15 * 0.15));
        atEdge = mix(atEdge, conf, siteW);
        float tLocal = tension * (1.0 + 1.5 * siteW);
        col = mix(col, water + bw * aT, (1.0 - pow(1.0 - 0.03, fs)) * tLocal * near * atEdge * dense);
        // uniform interiors only: slowly resolve two-ink mixtures (murk) to the nearest pure
        // ink or water -- never near an edge, so no posterised staircases
        vec3 nb = water;
        float nd = dot(col - water, col - water);
        for (int i = 0; i < 4; i++){
            vec3 ink = paletteInk(pal, i);
            float d = dot(col - ink, col - ink);
            if (d < nd){ nd = d; nb = ink; }
        }
        vec3 nbw = nb - water;
        vec3 nbT = water + nbw * clamp(dot(col - water, nbw) / max(dot(nbw, nbw), 1e-5), 0.75, 1.05);
        col = mix(col, nbT, (1.0 - pow(1.0 - 0.012, fs)) * separation * (1.0 - smoothstep(0.02, 0.08, grad)) * smoothstep(0.04, 0.10, bd)
                  * smoothstep(0.10, 0.22, length(col - water)));   // never amplify faint residue into ghosts
    }

    // ---- palette change is an EVENT: a dye front sweeps across the wall and re-dyes each
    //      pixel exactly once (the frame it crosses), old ink i -> new ink i, keeping the
    //      ink's density and grain. A one-shot map can't stall halfway, so no mud.
    {
        float cyc = floor(time / 36.0);
        float since = time - cyc * 36.0;
        if (cyc > 0.0 && since < 7.0){
            int palPrev = int(mod(cyc - 1.0, 3.0));
            float xr = uv.x + 0.25 * uv.y + 0.22 * dot_fbm(vec3(uv * 1.8, cyc));   // ragged, slightly diagonal front
            float front = -aspect * 1.5 + since / 6.0 * aspect * 3.0;
            float frontPrev = -aspect * 1.5 + (since - frameDt) / 6.0 * aspect * 3.0;
            if (xr <= front && xr > frontPrev){
                vec3 cw = col - water;
                float cl = length(cw);
                int bi = -1;
                float bc = 0.8;
                for (int i = 0; i < 4; i++){
                    vec3 ow = paletteInk(palPrev, i) - water;
                    float cs = dot(cw, ow) / (cl * length(ow) + 1e-5);
                    if (cs > bc){ bc = cs; bi = i; }
                }
                // skip anything already dyed (advection can carry it back across the front)
                float nc = -1.0;
                for (int i = 0; i < 4; i++){
                    vec3 nw = paletteInk(pal, i) - water;
                    nc = max(nc, dot(cw, nw) / (cl * length(nw) + 1e-5));
                }
                if (bi >= 0 && bc > nc){
                    vec3 ow = paletteInk(palPrev, bi) - water;
                    float a = clamp(dot(cw, ow) / dot(ow, ow), 0.0, 1.2);
                    vec3 residual = cw - ow * a;                                  // grain / translucency
                    col = clamp(water + (paletteInk(pal, bi) - water) * a + residual, 0.0, 1.0);
                }
            }
        }
    }

    // ink slowly dissolves back into the water
    col = mix(col, water, 1.0 - pow(0.9996, fs));
    // thin films evaporate faster than dense ink: faint residue never lingers as colour ghosts
    col = mix(col, water, (1.0 - pow(1.0 - 0.03, fs)) * (1.0 - smoothstep(0.04, 0.12, length(col - water))));
    outputColor = vec4(col * displayL(uv, aspect, time), 1.0);
}
