#version 150

uniform sampler2D tex;
uniform vec2 dimensions;
uniform float time;
uniform float audio[256];

// knobs
uniform float rainAmount;     // 0..2
uniform float dropSize;       // 0.5..2
uniform float focus;          // 0..1
uniform float cityGlow;       // 0..2
uniform float traffic;        // 0..2
uniform float streetlights;   // 0..2
uniform float kickPunch;      // 0..1
uniform float hueShift;       // 0..1
uniform float brightness;
uniform float bokehSize;      // 0.4..1.6
uniform float blades;         // 4 = round, 5..8 = polygonal aperture
uniform float neonMix;        // 0..2
uniform float colorTemp;      // -1 (cool) .. 1 (warm)
uniform float saturation;     // 0..2
uniform float interiorGlow;   // 0..1
uniform float trailLength;    // 0..2
uniform float windowFog;      // 0..1
uniform float fogWipe;        // 0..1
uniform float vignette;       // 0..1
uniform float grain;          // 0..1
uniform float direction;      // 0/1: which side window (mirrors the drive)

// CPU (integrated / analysed)
uniform float kickEnv, rainDrive, highLvl, live;
uniform float speed, wind, dropClock;
uniform float offFar, offMid, offNear, offCar;
uniform float wDown, wRes, wHwy, wTun;
uniform float sigRed, sigGreen;
uniform float sweepX, sweepAmp;
uniform float flash, flashX;     // CPU: lightning

in vec2 coord;
out vec4 outputColor;

#define PI 3.14159265359
#define S(a, b, x) smoothstep(a, b, x)

float h11(float p){ p = fract(p * .1031); p *= p + 33.33; p *= p + p; return fract(p); }
float h21(vec2 p){ vec3 p3 = fract(vec3(p.xyx) * .1031); p3 += dot(p3, p3.yzx + 33.33); return fract((p3.x + p3.y) * p3.z); }
vec3 h31(float p){ vec3 p3 = fract(vec3(p) * vec3(.1031, .1030, .0973)); p3 += dot(p3, p3.yzx + 33.33); return fract((p3.xxy + p3.yzz) * p3.zyx); }
vec3 h32(vec2 p){ vec3 p3 = fract(vec3(p.xyx) * vec3(.1031, .1030, .0973)); p3 += dot(p3, p3.yxz + 33.33); return fract((p3.xxy + p3.yzz) * p3.zyx); }
float n11(float x){ float i = floor(x), f = fract(x); return mix(h11(i), h11(i + 1.0), f * f * (3.0 - 2.0 * f)); }

vec3 hueRotate(vec3 c, float a){
    const vec3 k = vec3(0.57735);
    float ca = cos(a);
    return c * ca + cross(k, c) * sin(a) + k * dot(k, c) * (1.0 - ca);
}

// ---------------- area palettes (weights come from the CPU route) ----------------
struct Pal { vec3 sky0; vec3 sky1; vec3 warm; vec3 cool; vec3 neonA; vec3 neonB; };
Pal palette(){
    Pal d; // downtown: neon over cool LED, violet-blue sky
    d.sky0 = vec3(0.012, 0.010, 0.035); d.sky1 = vec3(0.09, 0.04, 0.11);
    d.warm = vec3(1.00, 0.55, 0.35); d.cool = vec3(0.55, 0.75, 1.00); d.neonA = vec3(1.0, 0.22, 0.65); d.neonB = vec3(0.20, 0.95, 0.90);
    Pal r; // residential: sodium, warm windows, low brown cloud
    r.sky0 = vec3(0.010, 0.010, 0.020); r.sky1 = vec3(0.10, 0.06, 0.035);
    r.warm = vec3(1.00, 0.58, 0.22); r.cool = vec3(0.75, 0.80, 0.95); r.neonA = vec3(1.0, 0.40, 0.20); r.neonB = vec3(0.45, 0.70, 1.00);
    Pal h; // highway: dark, sparse, blue-white LED lamps
    h.sky0 = vec3(0.004, 0.006, 0.014); h.sky1 = vec3(0.018, 0.024, 0.042);
    h.warm = vec3(1.00, 0.70, 0.40); h.cool = vec3(0.70, 0.85, 1.00); h.neonA = vec3(1.0, 0.30, 0.25); h.neonB = vec3(0.40, 0.80, 1.00);
    Pal p;
    float wd = wDown + wTun * 0.0, wr = wRes, wh = wHwy + wTun;
    float ws = max(wd + wr + wh, 1e-3);
    p.sky0 = (d.sky0 * wd + r.sky0 * wr + h.sky0 * wh) / ws;
    p.sky1 = (d.sky1 * wd + r.sky1 * wr + h.sky1 * wh) / ws;
    p.warm = (d.warm * wd + r.warm * wr + h.warm * wh) / ws;
    p.cool = (d.cool * wd + r.cool * wr + h.cool * wh) / ws;
    p.neonA = (d.neonA * wd + r.neonA * wr + h.neonA * wh) / ws;
    p.neonB = (d.neonB * wd + r.neonB * wr + h.neonB * wh) / ws;
    return p;
}

// ---------------- motion-blurred bokeh ----------------
// a lens disc smeared horizontally by L (a capsule); energy conserved across the smear
vec3 bokeh(vec2 p, vec2 c, float r, vec3 col, float L){
    r *= bokehSize;
    col /= max(bokehSize, 0.4);
    vec2 q = p - c;
    float hx = 0.5 * L;
    float dx = max(abs(q.x) - hx, 0.0);
    vec2 qq = vec2(sign(q.x) * dx, q.y);
    float d = length(qq);
    if (blades > 4.001){                                  // polygonal aperture (blades eased on the CPU: shapes morph, never snap)
        float ang = atan(qq.y, qq.x) + 0.3;
        float nA = floor(blades), fb = blades - nA;
        float kA = PI / max(nA, 5.0), kB = PI / max(nA + 1.0, 5.0);
        float pA = nA < 4.5 ? 1.0 : cos(mod(ang, 2.0 * kA) - kA) / cos(kA);
        float pB = cos(mod(ang, 2.0 * kB) - kB) / cos(kB);
        d *= mix(pA, pB, fb);
    }
    float aa = max(r * 0.07, 0.0015);
    vec3 dd = vec3(d * 0.985, d, d * 1.015);              // chromatic fringe
    vec3 disc = S(r + aa, r - aa, dd);
    float rim = 1.0 + 0.4 * S(0.55 * r, r, d) * S(0.5 * r, 0.0, hx);   // bright rim only when sharp-ish
    float halo = exp(-max(d - r, 0.0) / (r * 0.6)) * 0.1;
    float e = (2.0 * r) / (2.0 * r + L);
    return col * (disc * rim + halo) * e;
}

// a glowing lamp streak (soft core + glare), smeared horizontally by L; energy conserved
vec3 glowStreak(vec2 p, vec2 c, float r, vec3 col, float L){
    vec2 q = p - c;
    float d = length(vec2(max(abs(q.x) - 0.5 * L, 0.0), q.y));
    float e = (2.0 * r) / (2.0 * r + L);
    return col * (exp(-d * d / (r * r)) + 0.25 * exp(-d / (r * 2.5))) * e;
}

// one parallax layer of lights in a band; off = integrated offset, L = motion-blur length
vec3 lightLayer(vec2 p, float off, float cell, float rBase, float yLo, float yHi, float seed, float density,
                Pal P, float neonAmt, float kick, float blurK, float L){
    vec3 acc = vec3(0.0);
    vec2 q = vec2(p.x + off, p.y);
    vec2 id0 = floor(q / cell);
    float e = min(1.0 / (blurK * blurK), 4.0);
    for (int j = -1; j <= 1; j++)
    for (int i = -1; i <= 1; i++){
        vec2 id = id0 + vec2(i, j);
        vec3 n = h32(id + seed);
        vec2 c = (id + 0.2 + 0.6 * n.xy) * cell;
        float yw = S(yLo - 0.08, yLo, c.y) * S(yHi + 0.08, yHi, c.y);
        float on = S(n.z - 0.04, n.z + 0.04, density * yw);      // fades in/out as density changes: no pops
        if (on <= 0.0) continue;
        vec3 m = h32(id * 1.7 + seed + 3.1);
        float thr = 1.0 - 0.45 * (neonAmt * neonMix + 0.3 * max(neonMix - 1.0, 0.0));
        float nw = S(thr - 0.06, thr + 0.06, m.x);
        vec3 col = mix(m.z < 0.6 ? P.warm : P.cool, m.y < 0.5 ? P.neonA : P.neonB, nw);
        float lum = 0.12 + 0.88 * m.y * m.y * m.y;
        lum *= 0.85 + 0.15 * sin(time * (0.3 + m.z) + m.x * 40.0);
        lum *= 1.0 + kick * (0.6 + 0.9 * nw);
        float r = rBase * (0.7 + 0.6 * m.z) * blurK;
        // keep each light (smear included) inside the 3x3 cells searched, or it would pop at cell edges
        float kk = min(1.0, 1.05 * cell / (r * bokehSize * 1.4 + 0.5 * L));
        acc += bokeh(q, c, r * kk, col * lum * e * on, L * kk);
    }
    return acc;
}

float towerH(float b){ float tall = h11(b * 1.37); return 0.02 + 0.26 * tall * tall; }
float buildings(vec2 p, float blurK, float hScale){
    float u = p.x * 7.0, b0 = floor(u);
    float w = 0.04 * blurK * 7.0;
    float m = 0.0;
    for (int k = -1; k <= 1; k++){
        float b = b0 + float(k);
        float h = -0.06 + (towerH(b) + 0.03 * n11(b * 3.1)) * hScale;
        float xm = S(-w, w, u - b) * S(-w, w, b + 1.0 - u);
        float ym = S(h + 0.03 * blurK, h - 0.03 * blurK, p.y);
        m = max(m, xm * ym);
    }
    return m;
}

// traffic lane: pairs of lamps, smeared by their relative speed
vec3 lane(vec2 p, float y0, float off, float cell, vec3 col, float r, float seed, float blurK, float L, float dens){
    vec3 acc = vec3(0.0);
    float x = p.x + off;
    float id0 = floor(x / cell);
    float e = min(1.0 / (blurK * blurK), 4.0);
    for (int i = -1; i <= 1; i++){
        float id = id0 + float(i);
        vec3 n = h31(id + seed);
        float on = S(n.z - 0.03, n.z + 0.03, 0.3 * dens);
        if (on <= 0.0) continue;
        vec2 c = vec2((id + 0.5 + 0.6 * (n.x - 0.5)) * cell, y0 + (n.y - 0.5) * 0.015);
        float rr = r * (0.8 + 0.4 * n.y) * blurK;
        vec3 cc = col * (0.6 + 0.6 * n.x) * e * on;
        acc += bokeh(vec2(x, p.y), c, rr, cc, L);
        acc += bokeh(vec2(x, p.y), c + vec2(rr * 2.4, 0.0), rr, cc, L);
    }
    return acc;
}
vec3 laneStreaks(vec2 p, float y0, float off, float cell, vec3 col, float r, float seed, float blurK, float L, float dens){
    vec3 acc = vec3(0.0);
    float x = p.x + off;
    float id0 = floor(x / cell);
    float depth = (y0 - 0.02) - p.y;
    if (depth < 0.0) return acc;
    for (int i = -1; i <= 1; i++){
        float id = id0 + float(i);
        vec3 n = h31(id + seed);
        float on = S(n.z - 0.03, n.z + 0.03, 0.3 * dens);
        if (on <= 0.0) continue;
        float cx = (id + 0.5 + 0.6 * (n.x - 0.5)) * cell;
        float rr = r * (0.8 + 0.4 * n.y) * blurK;
        float w = rr * 0.6 + depth * 0.04 + 0.2 * L;
        float rip = 0.85 + 0.15 * (n11(p.y * 40.0 + n.x * 20.0 + time * 1.3) * 2.0 - 1.0);
        float fall = exp(-depth * 7.0) * rip;
        float d0 = x - cx, d1 = x - (cx + rr * 2.4);
        acc += col * (0.6 + 0.6 * n.x) * on * (exp(-d0 * d0 / (w * w)) + exp(-d1 * d1 / (w * w))) * fall;
    }
    return acc;
}

// downtown hero signs: a few big out-of-focus neon tube signs at middle distance
vec3 heroSigns(vec2 p, Pal P, float blurK, float L, float dens){
    vec3 acc = vec3(0.0);
    float cell = 1.1;
    float x = p.x + offMid;
    float id0 = floor(x / cell);
    for (int i = -1; i <= 1; i++){
        float id = id0 + float(i);
        vec3 n = h31(id * 3.3 + 71.0);
        float on = S(n.z - 0.05, n.z + 0.05, dens);
        if (on <= 0.0) continue;
        vec2 c = vec2((id + 0.3 + 0.4 * n.x) * cell, -0.02 + 0.16 * n.y);
        vec2 hs = h11(id * 1.9) > 0.5 ? vec2(0.035, 0.13) : vec2(0.15, 0.035);
        hs.x += 0.5 * L;
        vec2 dq = abs(vec2(x, p.y) - c) - hs;
        float sd = length(max(dq, 0.0)) + min(max(dq.x, dq.y), 0.0) - 0.012;
        float w = (0.006 + 0.022 * blurK) * bokehSize;
        float tube = exp(-abs(sd) / w);
        float fill = S(0.01, -0.03, sd) * 0.15;
        vec3 col = h11(id * 5.1) > 0.5 ? P.neonA : P.neonB;
        float hum = 0.9 + 0.1 * sin(time * (2.0 + n.y * 3.0) + id);
        acc += col * (tube * 0.75 + fill) * hum * on * (2.0 * w / (2.0 * w + L));
    }
    return acc;
}

// highway: reflective cat-eyes at the lane edge, trucks we pass, overhead gantry signs
vec3 highway(vec2 p, vec3 col, Pal P, float blurK, float Lnear, float g){
    // lane cat-eyes streaming past low in the frame
    float x = p.x + offNear * 0.75;
    float ce = 0.16, cid = floor(x / ce);
    col += glowStreak(vec2(x - (cid + 0.5) * ce, p.y), vec2(0.0, -0.43), 0.006 * blurK + 0.003, vec3(1.0, 0.62, 0.18) * 0.55 * wHwy * g, Lnear * 0.75);
    // trucks in the next lane: a dark body with a row of amber marker lights and red tail lights
    float tx = p.x + offMid * 0.55 + time * 0.05;
    float tc = 2.6, tid = floor(tx / tc);
    for (int i = -1; i <= 0; i++){
        float id = tid + float(i);
        vec3 n = h31(id * 7.7 + 5.0);
        float on = S(n.z - 0.05, n.z + 0.05, 0.6 * traffic);
        if (on <= 0.0) continue;
        float cx = (id + 0.55) * tc;
        vec2 q = vec2(tx - cx, p.y + 0.20);
        float body = S(0.5 + 0.03 * blurK, 0.5 - 0.03 * blurK, abs(q.x)) * S(0.1 + 0.03 * blurK, 0.1 - 0.03 * blurK, abs(q.y));
        col = mix(col, P.sky0 * 0.3, body * 0.85 * wHwy * on);
        for (int k = 0; k < 6; k++){
            vec2 lc = vec2(cx - 0.45 + 0.18 * float(k), -0.10);
            col += bokeh(vec2(tx, p.y), lc, 0.011 * blurK, vec3(1.0, 0.55, 0.12) * 0.8 * wHwy * g * on, 0.0);
        }
        col += bokeh(vec2(tx, p.y), vec2(cx - 0.47, -0.27), 0.016 * blurK, vec3(1.0, 0.1, 0.05) * wHwy * g * on, 0.0);
    }
    // overhead gantries: a green sign board with white lettering, lit from below, passing overhead
    float gx = p.x + offNear * 0.55;
    float gc = 3.2, gid = floor(gx / gc + 0.5);
    if (h11(gid * 2.3) > 0.3){
        float f = gx - gid * gc;
        float hw = 0.32 + 0.5 * Lnear * 0.55;
        float soft = 0.012 * blurK + 0.25 * Lnear * 0.55;
        float board = S(hw + soft, hw - soft, abs(f)) * S(0.08 + soft, 0.08 - soft, abs(p.y - 0.31));
        float beam = S(0.012 + soft, 0.0, abs(p.y - 0.41)) * S(hw + 0.3, hw, abs(f));
        float letters = S(0.5, 0.9, n11(f * 38.0 + gid * 7.0)) * (S(0.012, 0.004, abs(p.y - 0.335)) + S(0.012, 0.004, abs(p.y - 0.29)));
        col = mix(col, P.sky0 * 0.2, beam * 0.8 * wHwy);
        col += (vec3(0.05, 0.32, 0.16) * board + vec3(0.9) * letters * board * S(hw, hw - 0.05, abs(f)) * 0.5) * wHwy * g;
    }
    return col;
}

// ---------------- the world outside the side window ----------------
vec3 outside(vec2 p, float blurK, Pal P, float kick){
    float g = cityGlow;
    float spd = speed;
    // sky + city glow on the cloud
    vec3 col = mix(P.sky1, P.sky0, S(-0.2, 0.5, p.y));
    float cityness = wDown + 0.6 * wRes + 0.25 * wHwy;
    col += P.warm * 0.08 * cityness * exp(-abs(p.y - 0.05) * 3.5);
    float cl = n11(p.x * 2.0 + offFar * 3.0 + time * 0.01) * 0.6 + n11(p.x * 5.3 + offFar * 5.0 + p.y * 3.0) * 0.4;
    col += P.warm * 0.03 * cityness * cl * S(0.5, 0.05, p.y) * S(-0.05, 0.15, p.y);

    // lightning: sheet light in the cloud, brightest toward the strike
    if (flash > 0.001){
        float fl = flash * (1.0 - wTun);
        col += vec3(0.62, 0.70, 0.95) * fl * (0.30 * exp(-abs(p.x - flashX) * 1.3) * (0.6 + cl) * S(-0.15, 0.45, p.y) + 0.04);
    }
    // far: skyline (downtown towers, residential roofs, highway nothing much)
    vec2 pf = vec2(p.x + offFar, p.y);
    float hScale = wDown * 1.0 + wRes * 0.35 + wHwy * 0.15;
    float bld = buildings(pf, blurK, hScale);
    col = mix(col, P.sky0 * 0.35, bld * 0.85);
    // hills along the highway
    float hill = -0.06 + 0.07 * n11(pf.x * 1.7) + 0.03 * n11(pf.x * 5.1);
    col = mix(col, P.sky0 * 0.4, wHwy * S(hill + 0.02, hill - 0.02, p.y));
    float farDens = 0.32 * wDown + 0.35 * wRes + 0.1 * wHwy;
    col += lightLayer(p, offFar, 0.045, 0.012, -0.24, 0.22 * hScale + 0.02, 1.0, farDens, P, wDown * 0.4, kick * 0.3, blurK, 0.0) * 0.7 * g;

    // mid: shopfronts, signs, porch lights (neon downtown)
    float vMid = spd * 0.38;
    float Lmid = vMid * 0.07;
    float midDens = 0.22 * wDown + 0.3 * wRes + 0.06 * wHwy;
    col += lightLayer(p, offMid, 0.13, 0.034, -0.42, 0.08, 7.0, midDens, P, wDown, kick * 0.6, blurK, Lmid) * 0.75 * g;
    if (wDown > 0.01) col += heroSigns(p, P, blurK, Lmid, 0.75 * wDown * min(neonMix, 1.5)) * (0.9 + 0.6 * kick) * g;

    // road: oncoming headlights (fast smears) and same-direction tail lights drifting
    float Lcar = (spd * 0.9 + 0.12) * 0.14;
    col += lane(p, -0.31, offCar, 0.24, P.cool * vec3(1.0, 0.97, 0.9) + 0.2, 0.020, 21.0, blurK, Lcar, traffic) * 0.7 * g;
    col += lane(p, -0.275, offMid * 0.25 + time * 0.03, 0.31, vec3(1.0, 0.10, 0.06), 0.016, 33.0, blurK, 0.01, traffic) * 0.6 * g;
    if (p.y < -0.27){
        col += (laneStreaks(p, -0.31, offCar, 0.24, P.cool * vec3(1.0, 0.97, 0.9) + 0.2, 0.020, 21.0, blurK, Lcar, traffic) * 0.7
              + laneStreaks(p, -0.275, offMid * 0.25 + time * 0.03, 0.31, vec3(1.0, 0.10, 0.06), 0.016, 33.0, blurK, 0.01, traffic) * 0.6) * 0.3 * g;
    }

    // near: roadside lamps and lit signs whipping by, heavily smeared; dark posts crossing the view
    float vNear = spd * 1.7;
    float Lnear = vNear * 0.16;
    col += lightLayer(p, offNear, 0.55, 0.12, -0.55, 0.45, 13.0, 0.2 + 0.15 * wDown, P, wDown, kick, blurK * 1.2, Lnear) * 0.32 * g;
    {
        float x = p.x + offNear * 1.15;
        float cell = 0.9, id = floor(x / cell);
        float px = (id + 0.5) * cell + (h11(id * 3.7) - 0.5) * 0.3;
        float wpost = 0.035 * blurK + 0.5 * Lnear * 1.15;
        float post = S(wpost, wpost * 0.4, abs(x - px)) * step(0.45, h11(id * 9.1)) * (1.0 - wTun);
        col *= 1.0 - 0.75 * post * (0.035 * blurK) / (0.035 * blurK + 0.5 * Lnear * 1.15);
    }

    if (wHwy > 0.01) col = highway(p, col, P, blurK, Lnear, g);
    // overhead streetlights: a warm wash sweeping across the glass as each lamp passes
    float lampSpace = 1.6;
    float lp = (p.x + offNear) / lampSpace;
    float lid = floor(lp + 0.5);
    float lx = (lp - lid) * lampSpace;                     // signed distance to the nearest lamp (screen units)
    float lampOn = step(0.25, h11(lid * 5.3)) * (0.6 * wRes + 0.5 * wDown + 0.7 * wHwy) * streetlights;
    vec3 lampCol = mix(P.warm, P.cool, wHwy * 0.7);
    col += lampCol * lampOn * 0.13 * exp(-lx * lx * 2.2) * S(-0.6, 0.5, p.y);
    col += bokeh(vec2(lx, p.y), vec2(0.0, 0.44), 0.065 * blurK, lampCol * lampOn * 1.3, Lnear);

    // tunnel: rhythmic sodium lamps along the ceiling, wet walls catching the light
    if (wTun > 0.001){
        vec3 tcol = vec3(0.012, 0.010, 0.008);
        float tx = p.x + offNear * 0.8;
        float tl = 0.62;
        float tid = floor(tx / tl);
        float tf = (tx - (tid + 0.5) * tl);
        vec3 sod = vec3(1.0, 0.52, 0.12);
        float Lt = spd * 1.36 * 0.16 + 0.02;
        tcol += glowStreak(vec2(tf, p.y), vec2(0.0, 0.37), 0.04 * blurK, sod * 1.5, Lt);
        // wall panels streaming past, each lit by the lamp above it
        float px2 = (p.x + offNear * 0.8) / 0.21;
        float seam = S(0.06, 0.0, abs(fract(px2) - 0.5) - 0.44) * 0.5 + 0.5;
        float wash = 0.55 + 0.45 * cos(tx / tl * 2.0 * PI);
        tcol += sod * 0.10 * wash * seam * S(0.45, 0.05, abs(p.y - 0.02)) * (0.8 + 0.2 * n11(px2 * 0.37));
        // a row of small green exit/marker lights along the wall
        float mx = p.x + offNear * 0.8;
        float mid2 = floor(mx / 1.3);
        tcol += glowStreak(vec2(mx - (mid2 + 0.5) * 1.3, p.y), vec2(0.0, -0.12), 0.014 * blurK, vec3(0.15, 1.0, 0.45) * 0.7, Lt);
        tcol += sod * 0.06 * wash * exp(-abs(p.y + 0.30) * 10.0);     // wet road sheen
        col = mix(col, tcol * g, wTun);
    }

    // red light: the signal hangs above the junction; red, then green
    float sig = sigRed + sigGreen;
    if (sig > 0.001){
        vec3 sc = vec3(1.0, 0.08, 0.04) * sigRed + vec3(0.1, 1.0, 0.45) * sigGreen;
        col += bokeh(p, vec2(0.48, 0.33), 0.06 * blurK, sc * 1.4, 0.0);
        col += sc * 0.05 * exp(-length(p - vec2(0.48, 0.33)) * 3.0);
    }

    // kick: an overtaking car's headlights sweep along the glass
    if (sweepAmp > 0.001){
        vec3 hc = vec3(0.95, 0.97, 1.0);
        float Ls = 0.12;
        col += glowStreak(p, vec2(sweepX, -0.18), 0.06 * blurK, hc * sweepAmp * 0.9, Ls);
        col += glowStreak(p, vec2(sweepX - 0.24, -0.18), 0.06 * blurK, hc * sweepAmp * 0.9, Ls);
        col += hc * sweepAmp * 0.045 * exp(-pow((p.x - sweepX + 0.1) * 1.6, 2.0)) * kickPunch;
    }
    return col;
}

// ---------------- rain on the glass (airflow shears the flow back along the window) ----------------
float saw(float b, float t){ return S(0.0, b, t) * S(1.0, b, t); }

vec2 slideLayer(vec2 uv, float t, float amt){
    vec2 UV = uv;
    uv.y += t * 0.75;
    vec2 a = vec2(6.0, 1.0);
    vec2 grid = a * 2.0;
    vec2 id = floor(uv * grid);
    uv.y += h11(id.x * 7.13);
    id = floor(uv * grid);
    vec3 n = h31(id.x * 35.2 + id.y * 2376.1);
    float on = S(n.y - 0.08, n.y + 0.08, amt);
    vec2 st = fract(uv * grid) - vec2(0.5, 0.0);
    float x = n.x - 0.5;
    float y = UV.y * 20.0;
    x += sin(y + sin(y)) * (0.5 - abs(x)) * (n.z - 0.5);
    x *= 0.7;
    float ti = fract(t + n.z);
    y = (saw(0.85, ti) - 0.5) * 0.9 + 0.5;
    vec2 p = vec2(x, y);
    float d = length((st - p) * a.yx);
    float drop = S(0.4, 0.0, d);
    float r = sqrt(S(1.0, y, st.y));
    float cd = abs(st.x - x);
    float trail = S(0.23 * r, 0.15 * r * r, cd);
    float front = S(-0.02, 0.02, st.y - y);
    trail *= front * r * r;
    float yy = fract(UV.y * 10.0) + (st.y - 0.5);
    float beads = S(0.3, 0.0, length(st - vec2(x, yy)));
    return vec2(drop + beads * r * front * min(trailLength, 1.6), trail * trailLength) * on;
}

float staticDrops(vec2 uv, float t, float amt){
    uv *= 30.0;
    vec2 id = floor(uv);
    uv = fract(uv) - 0.5;
    vec3 n = h32(id * vec2(107.45, 3543.654));
    vec2 p = (n.xy - 0.5) * 0.36;
    float d = length(uv - p);
    float fade = saw(0.025, fract(t + n.z));
    float on = S(n.x * 0.9, n.x * 0.9 + 0.1, amt);
    return S(0.3, 0.0, d) * fract(n.z * 10.0) * fade * on;
}

vec3 drops(vec2 uv, float t, float l0, float l1, float l2){
    float s = staticDrops(uv, t, l0);
    vec2 m1 = slideLayer(uv, t, l1);
    vec2 m2 = slideLayer(uv * 1.85, t, l2);
    float raw = s + m1.x + m2.x;
    return vec3(S(0.3, 1.0, raw), max(m1.y, m2.y), raw);
}

void main(){
    vec2 res = dimensions;
    vec2 pc = vec2(coord.x, res.y - coord.y);          // up-is-up (in-app coord.y = 0 is the TOP)
    vec2 p = (pc - 0.5 * res) / res.y;
    if (direction > 0.5) p.x = -p.x;                   // the other side window: the whole drive mirrors
    Pal P = palette();
    float kick = kickEnv * kickPunch;

    // rain amount: mids (CPU) x knob; at speed the airflow clears some of the glass
    float amt = clamp(rainAmount * (0.55 + 0.6 * rainDrive) * (1.0 - 0.25 * S(0.6, 1.6, wind)), 0.0, 1.6);
    // fog wipe: a hand-wiped clear patch (fewer drops, no condensation), with a streaky edge
    float wipeM = 0.0;
    if (fogWipe > 0.001){
        vec2 wq = (p - vec2(-0.32, -0.03)) / (vec2(0.78, 0.34) * (0.25 + 0.75 * fogWipe));
        float wr = length(wq);
        float streak = 0.06 * (n11(atan(wq.y, wq.x) * 9.0) - 0.5) + 0.03 * sin(wq.y * 40.0);
        wipeM = S(1.0 + streak, 0.82 + streak, wr) * S(0.0, 0.15, fogWipe);
        amt *= 1.0 - 0.75 * wipeM;
    }
    float l0 = S(0.0, 0.6, amt), l1 = S(0.1, 0.75, amt), l2 = S(0.3, 1.1, amt);

    // airflow: shear the drop field so drops run diagonally back along the glass (scene moves left,
    // air pushes the water toward the rear = left); beads stretch along the flow
    float shear = 1.4 * wind;
    vec2 duv = p / dropSize * 0.5 * (1.0 + 0.5 * wind);
    duv.x = duv.x * (1.0 + 0.2 * wind) - duv.y * shear;
    vec3 c = drops(duv, dropClock, l0, l1, l2);
    vec2 e = vec2(0.0016, 0.0);
    float cx = drops(duv + e, dropClock, l0, l1, l2).z;
    float cy = drops(duv + e.yx, dropClock, l0, l1, l2).z;
    vec2 nrm = vec2(cx - c.z, cy - c.z);

    float dropM = S(0.05, 0.35, c.x);
    float clear = max(max(dropM, c.y * 0.8), wipeM);
    float blurMain = mix(1.0, 0.45, focus) * (1.0 + 0.12 * kick);
    float blurK = mix(blurMain * (1.0 + 0.85 * windowFog), blurMain, clear);

    vec3 col = outside(p, blurK, P, kick);
    // light falling on the glass right now (lamps + sweeps): drops sparkle as it passes
    float lampLight = 0.0;
    {
        float lp = (p.x + offNear) / 1.6;
        float lx = (lp - floor(lp + 0.5)) * 1.6;
        lampLight = exp(-lx * lx * 2.2) * step(0.25, h11(floor(lp + 0.5) * 5.3)) * (0.6 * wRes + 0.5 * wDown + 0.7 * wHwy) * streetlights;
        lampLight += 0.5 * sweepAmp * kickPunch * exp(-pow((p.x - sweepX + 0.1) * 1.6, 2.0));
        lampLight += wTun * (0.5 + 0.5 * cos((p.x + offNear * 0.8) / 0.62 * 2.0 * PI)) * 0.5;
        lampLight += flash * (1.0 - wTun) * 0.7;
    }
    if (dropM > 0.001){
        // a bead: displaced, flipped, nearly-focused view, gathering light
        vec2 q = p - nrm * 5.0 * dropSize;
        vec3 sharp = outside(q, blurMain * 0.8, P, kick) * 1.5;
        vec2 gn = nrm / (length(nrm) + 1e-4);
        float edge = S(0.2, 0.7, length(nrm) * 40.0);
        float cres = S(0.0, 0.9, gn.y) * edge * S(0.3, 0.7, c.x);
        float lumA = dot(sharp, vec3(0.3, 0.59, 0.11));
        sharp += (sharp + lumA * 0.5 + P.sky1 * 0.4) * cres * 1.4;
        // glassy rim: catches the ambient city glow and any passing light
        sharp += (P.sky1 * 0.2 + mix(P.warm, vec3(1.0), 0.5) * lampLight * 0.25) * edge * 0.5;
        sharp *= 1.0 - 0.3 * S(0.6, 0.15, c.x);
        col = mix(col, sharp, dropM);
    }
    // wet trails: thin water film streaks that catch the light
    float trailV = clamp(c.y, 0.0, 1.0) * (1.0 - dropM);
    col = mix(col, col * 1.3 + P.sky1 * 0.25 + vec3(lampLight * 0.05), trailV * 0.55);
    // a wet film on the glass (the fog) - light diffuses in it
    float fog = (1.0 - clear) * (0.55 + 0.45 * n11(p.x * 3.0 + 10.0) * n11(p.y * 4.0 + 3.0 + p.x));
    float lum = dot(col, vec3(0.3, 0.59, 0.11));
    col = mix(col, col * 0.8 + lum * 0.3 + P.sky1 * 0.25 + vec3(0.012), fog * 0.62 * windowFog);

    // glints: a fixed key light plus the passing light, brighter with the highs
    vec3 N = normalize(vec3(-nrm * 40.0, 1.0));
    float spec = pow(max(dot(reflect(-normalize(vec3(-0.4, 0.6, 0.7)), N), vec3(0, 0, 1)), 0.0), 60.0);
    float spec2 = pow(max(dot(reflect(-normalize(vec3(0.0, 0.8, 0.6)), N), vec3(0, 0, 1)), 0.0), 30.0);
    col += dropM * (spec * (0.3 + 0.9 * highLvl) * mix(P.warm, vec3(1.0), 0.5) * 0.5 + spec2 * lampLight * 1.2 * mix(P.warm, vec3(1.0), 0.4));
    // highs: a fine spatter of beads catching the light
    vec2 mg = vec2(p.x * (1.0 + 0.5 * wind) + p.y * shear * 0.3, p.y) * 140.0;
    vec2 mi = floor(mg); vec2 mf = fract(mg) - 0.5;
    vec3 mh = h32(mi + 17.0);
    float on = S(mh.z - 0.03, mh.z + 0.03, 0.02 + highLvl * 0.10);
    float bead = S(0.2, 0.04, length(mf - (mh.xy - 0.5) * 0.5)) * on;
    float lumB = dot(col, vec3(0.3, 0.59, 0.11));
    col += bead * (S(0.04, 0.2, lumB) * col + lampLight * 0.15) * (0.5 + 0.7 * highLvl);

    col *= 0.6 + 0.4 * live;

    // interior glow: the dashboard and a phone screen reflected on the inside of the glass
    if (interiorGlow > 0.001){
        vec3 ig = vec3(0.0);
        ig += vec3(1.0, 0.55, 0.2) * 0.05 * exp(-(p.y + 0.5) * 7.0) * S(1.1, 0.1, abs(p.x - 0.35));     // dash glow
        ig += vec3(0.25, 0.75, 0.9) * 0.035 * exp(-(p.y + 0.5) * 12.0) * S(0.5, 0.0, abs(p.x - 0.55));  // instrument light
        for (int k = 0; k < 2; k++){                                                                    // dial rings
            vec2 dc = vec2(0.42 + 0.17 * float(k), -0.38);
            float rr = abs(length(p - dc) - 0.06);
            ig += vec3(0.35, 0.8, 1.0) * 0.05 * exp(-rr * rr / 0.00006) * S(-0.2, 0.3, (p - dc).y + 0.03);
        }
        vec2 ph = p - vec2(-0.55, -0.33);                                                               // phone
        vec2 pq = abs(vec2(ph.x * cos(0.25) - ph.y * sin(0.25), ph.x * sin(0.25) + ph.y * cos(0.25))) - vec2(0.05, 0.095);
        float psd = length(max(pq, 0.0)) + min(max(pq.x, pq.y), 0.0) - 0.012;
        ig += vec3(0.75, 0.82, 1.0) * (0.045 * S(0.02, -0.01, psd) * (0.85 + 0.15 * n11(time * 0.3)) + 0.02 * exp(-max(psd, 0.0) * 25.0));
        col += ig * interiorGlow * 3.2;
    }

    float vig = S(1.15, 0.2, length(p * vec2(0.85, 1.0)));
    col *= mix(1.0 - vignette, 1.0, vig);
    col = hueRotate(col, hueShift * 2.0 * PI);
    // grade: colour temperature and saturation
    col *= vec3(1.0 + 0.22 * colorTemp, 1.0 + 0.03 * colorTemp, 1.0 - 0.25 * colorTemp);
    float gl = dot(col, vec3(0.3, 0.59, 0.11));
    col = max(mix(vec3(gl), col, saturation), 0.0);
    col = 1.0 - exp(-max(col, 0.0) * brightness * 1.4);
    col = pow(col, vec3(0.95));
    // film grain: small, luminance-weighted, changes at 24 fps
    if (grain > 0.001){
        float gn = h21(pc + floor(time * 24.0) * vec2(37.1, 17.3)) + h21(pc * 1.7 + floor(time * 24.0) * 3.1) - 1.0;
        col += gn * 0.06 * grain * (0.35 + 0.65 * sqrt(max(dot(col, vec3(0.33)), 0.0)));
    }
    outputColor = vec4(col, 1.0);
}
