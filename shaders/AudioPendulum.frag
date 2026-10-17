#version 150

uniform sampler2D tex;
uniform vec2 dimensions;
uniform float time;
uniform float audio[256];

// CPU (pendulum-wave physics, see AudioPendulumShader.hpp)
uniform float angles[20];
uniform float lengths[20];
uniform float count;
uniform float align;      // 0..1: how lined-up the row is (1 = the re-alignment moment)
uniform float kickEnv;
uniform float sparkle;    // highs onset envelope
uniform float warmth;     // mids, smoothed
uniform float orbit;      // slow camera orbit offset (rad), smoothed on the CPU
uniform float live;

// knobs
uniform float cameraDrift;
uniform float keyLight;
uniform float depthOfField;
uniform float haze;
uniform float hueShift;
uniform float brightness;

in vec2 coord;
out vec4 outputColor;

#define PI 3.14159265359
const float H = 1.98;     // pivot height (the beam)
const float R = 0.13;     // bob radius
const float ROW = 4.4;    // row length

int N;
vec3 piv[20];
vec3 bob[20];

// ---------------- lighting setups (crossfaded over minutes) ----------------
vec3 keyDir, keyCol, rimDir, rimCol, ambCol, wallCol, matTint;
float keySize, spotAmt, stripAmt; vec3 stripCol;

void setupLights(int k, out vec3 kd, out vec3 kc, out vec3 rd, out vec3 rc, out vec3 am, out vec3 wc, out vec3 mt, out float ks, out float sp){
    if (k == 0){          // warm gallery: big amber softbox upper-left-front, cool rim, brass
        kd = normalize(vec3(-0.65, 0.80, 0.55)); kc = vec3(1.00, 0.72, 0.46) * 3.6;
        rd = normalize(vec3(0.75, 0.35, -0.80)); rc = vec3(0.40, 0.60, 1.00) * 2.4;
        am = vec3(0.030, 0.023, 0.018); wc = vec3(0.034, 0.024, 0.019); mt = vec3(1.00, 0.80, 0.52);
        ks = 0.42; sp = 0.0;
    } else if (k == 1){   // cold blue night: moonlight from behind-right, a low warm practical, chrome
        kd = normalize(vec3(0.55, 0.70, -0.25)); kc = vec3(0.50, 0.66, 1.00) * 3.4;
        rd = normalize(vec3(-0.85, 0.12, 0.55)); rc = vec3(1.00, 0.55, 0.25) * 0.9;
        am = vec3(0.020, 0.028, 0.050); wc = vec3(0.018, 0.028, 0.055); mt = vec3(0.86, 0.90, 0.97);
        ks = 0.30; sp = 0.0;
    } else {              // single hard theatre spotlight from above
        kd = normalize(vec3(0.08, 1.0, 0.22)); kc = vec3(1.00, 0.92, 0.80) * 4.5;
        rd = normalize(vec3(0.2, 0.25, -1.0)); rc = vec3(0.30, 0.35, 0.45) * 0.35;
        am = vec3(0.006); wc = vec3(0.008, 0.008, 0.010); mt = vec3(0.95, 0.93, 0.90);
        ks = 0.10; sp = 1.0;
    }
}
void blendLights(float t){
    float e = t / 42.0;
    float f = fract(e);
    int i0 = int(mod(floor(e), 3.0)), i1 = int(mod(floor(e) + 1.0, 3.0));
    float b = smoothstep(0.78, 1.0, f);
    // ceiling strip lights: warm studio / cool night / off for the spotlight
    vec3 sc0 = vec3(1.0, 0.86, 0.7) * 1.4, sc1 = vec3(0.55, 0.7, 1.0) * 0.8, sc2 = vec3(0.0);
    vec3 sca = i0 == 0 ? sc0 : (i0 == 1 ? sc1 : sc2), scb = i1 == 0 ? sc0 : (i1 == 1 ? sc1 : sc2);
    stripCol = mix(sca, scb, b); stripAmt = 1.0;
    vec3 a1, a2, a3, a4, a5, a6, a7, b1, b2, b3, b4, b5, b6, b7; float a8, a9, b8, b9;
    setupLights(i0, a1, a2, a3, a4, a5, a6, a7, a8, a9);
    setupLights(i1, b1, b2, b3, b4, b5, b6, b7, b8, b9);
    keyDir = normalize(mix(a1, b1, b)); keyCol = mix(a2, b2, b); rimDir = normalize(mix(a3, b3, b)); rimCol = mix(a4, b4, b);
    ambCol = mix(a5, b5, b); wallCol = mix(a6, b6, b); matTint = mix(a7, b7, b); keySize = mix(a8, b8, b); spotAmt = mix(a9, b9, b);
}

vec3 horizonCol(){ return wallCol * 0.8 + keyCol * 0.003 * keyLight * (1.0 - spotAmt); }
float hash13(vec3 p){ p = fract(p * 0.1031); p += dot(p, p.zyx + 31.32); return fract((p.x + p.y) * p.z); }

// key light reaching a world point: broad pool for softboxes, a tight cone for the spotlight
float keyReach(vec3 p){
    vec2 c = vec2(0.0, 0.2);
    float pool = exp(-dot(p.xz - c, p.xz - c) / 9.0);
    float cone = smoothstep(1.9, 1.1, length(p.xz - c) + 0.15 * max(p.y - 1.0, 0.0));
    return mix(pool, cone, spotAmt) * keyLight * (1.0 + 0.22 * kickEnv);
}

// environment seen in reflections: rectangular key softbox, a rim strip, dark gallery, dim floor
vec3 env(vec3 d, float strip){
    vec3 col;
    if (d.y < 0.0){
        col = wallCol * 0.35 + keyCol * 0.004 * spotAmt;            // the floor, far away
        col += wallCol * 1.5 * exp(-abs(d.y) * 30.0);                // horizon line
    } else {
        col = wallCol * (0.6 + 0.6 * (1.0 - d.y));
        // three long ceiling strips running along the row: elegant curved highlights on every sphere
        if (d.y > 0.25){
            float q = d.z / d.y, lx = abs(d.x / d.y);
            float st = 0.0;
            for (int k = -1; k <= 1; k++) st += smoothstep(0.035, 0.012, abs(q - 0.55 * float(k) - 0.15));
            col += stripCol * st * smoothstep(1.6, 1.1, lx) * (1.0 + 0.8 * sparkle);
        }
    }
    // key softbox (rect in the light's frame)
    vec3 u = normalize(cross(keyDir, vec3(0.0, 0.0, 1.0) + vec3(0.001)));
    vec3 v = cross(u, keyDir);
    float cd = dot(d, keyDir);
    if (cd > 0.0){
        vec2 q = vec2(dot(d, u), dot(d, v)) / cd;
        vec2 sz = vec2(keySize, keySize * 0.62);
        vec2 e = abs(q) - sz;
        float box = smoothstep(0.06 * keySize + 0.004, 0.0, max(e.x, e.y));
        float core = 1.0 - 0.35 * smoothstep(0.0, 1.0, length(q / sz));   // brighter centre
        col += keyCol * box * core * (2.2 + 1.5 * sparkle) * keyLight;
        col += keyCol * 0.06 * exp(-length(q) * 3.0) * keyLight;            // glow around it
    }
    // rim strip
    float cr = dot(d, rimDir);
    if (cr > 0.0){
        vec3 ru = normalize(cross(rimDir, vec3(0.0, 1.0, 0.0)));
        float w = abs(dot(d, ru)) / cr;
        float hgt = abs(dot(d, cross(ru, rimDir))) / cr;
        col += rimCol * smoothstep(0.05, 0.02, w) * smoothstep(0.9, 0.5, hgt) * 1.6 * strip;
    }
    return col;
}

// soft shadow of a sphere (iq)
float sphSoftShadow(vec3 ro, vec3 rd, vec3 c, float r, float k){
    vec3 oc = ro - c;
    float b = dot(oc, rd);
    float cc = dot(oc, oc) - r * r;
    float h = b * b - cc;
    float d = sqrt(max(0.0, r * r - h)) - r;
    float t = -b - sqrt(max(h, 0.0));
    return (t > 0.0) ? smoothstep(0.0, 1.0, 2.5 * k * d / t) : 1.0;
}
float shadowAt(vec3 p){
    float s = 1.0;
    float k = mix(2.2, 7.0, spotAmt);        // softbox = soft, spotlight = crisper
    for (int i = 0; i < 20; i++){ if (i >= N) break; s *= sphSoftShadow(p, keyDir, bob[i], R, k); }
    return s;
}

// hard sphere hit (used in reflections)
float hitSphere(vec3 ro, vec3 rd, vec3 c, float r){
    vec3 oc = ro - c; float b = dot(oc, rd); float h = b * b - dot(oc, oc) + r * r;
    if (h < 0.0) return -1.0;
    float t = -b - sqrt(h);
    return t > 1e-3 ? t : -1.0;
}

vec3 floorShade(vec3 p, vec3 rd, bool withReflection);

// a sphere seen in a reflection: env reflection only (no further bounce)
vec3 sphereFlat(vec3 p, vec3 n, vec3 rd){
    vec3 rr = reflect(rd, n);
    float fr = 0.6 + 0.4 * pow(1.0 - max(dot(n, -rd), 0.0), 5.0);
    vec3 e = rr.y < 0.0 ? wallCol * 0.4 + keyCol * 0.03 * keyReach(p) : env(rr, 1.0);
    return e * matTint * fr * 0.85;
}

vec3 traceReflect(vec3 ro, vec3 rd, int skip){
    float tb = 1e9; int hi = -1;
    for (int i = 0; i < 20; i++){
        if (i >= N) break;
        if (i == skip) continue;
        float t = hitSphere(ro, rd, bob[i], R);
        if (t > 0.0 && t < tb){ tb = t; hi = i; }
    }
    float tf = rd.y < 0.0 ? -ro.y / rd.y : 1e9;
    if (hi >= 0 && tb < tf){
        vec3 p = ro + rd * tb; return sphereFlat(p, normalize(p - bob[hi]), rd);
    }
    if (tf < 1e8) return floorShade(ro + rd * tf, rd, false);
    return env(rd, 1.0);
}

vec3 floorShade(vec3 p, vec3 rd, bool withReflection){
    float kr = keyReach(p);
    float sh = shadowAt(p + vec3(0.0, 1e-3, 0.0));
    float ndl = max(keyDir.y, 0.0);
    vec3 base = vec3(0.030, 0.028, 0.026);
    vec3 col = base * (keyCol * ndl * kr * sh * 1.2 + ambCol * 6.0);
    // fine brushed grain
    col *= 0.93 + 0.07 * hash13(floor(p * vec3(180.0, 1.0, 9.0)));
    if (withReflection){
        vec3 rr = reflect(rd, vec3(0.0, 1.0, 0.0));
        float fr = 0.05 + 0.55 * pow(1.0 - max(-rd.y, 0.0), 5.0);
        // glossy: spheres reflected with soft coverage, blur grows with height above the floor
        vec3 refl = env(rr, 0.0) * 0.35;
        float best = 1e9; vec3 sc = vec3(0.0); float cov = 0.0;
        for (int i = 0; i < 20; i++){
            if (i >= N) break;
            vec3 oc = bob[i] - p; float tc = dot(oc, rr);
            if (tc <= 0.0) continue;
            float d = length(p + rr * tc - bob[i]);
            float w = 0.02 + 0.16 * tc;
            float c = smoothstep(R + w, R - w, d);
            if (c > 0.002 && tc < best){
                best = tc; cov = c;
                vec3 oc2 = p - bob[i]; float b2 = dot(oc2, rr); float h2 = b2 * b2 - dot(oc2, oc2) + R * R;
                vec3 q = h2 > 0.0 ? p + rr * (-b2 - sqrt(h2)) : bob[i] + normalize(p + rr * tc - bob[i]) * R;
                sc = sphereFlat(q, normalize(q - bob[i]), rr);
            }
        }
        refl = mix(refl, sc * 0.7, cov);
        col += refl * fr;
    }
    return col;
}

vec3 sphereShade(int i, vec3 p, vec3 n, vec3 rd){
    vec3 rr = reflect(rd, n);
    float fr = 0.55 + 0.45 * pow(1.0 - max(dot(n, -rd), 0.0), 5.0);
    vec3 refl = traceReflect(p + n * 2e-3, rr, i);
    vec3 col = refl * matTint * fr;
    // a breath of diffuse: polished brass is never a perfect mirror
    col += matTint * keyCol * max(dot(n, keyDir), 0.0) * 0.035 * (1.0 - 0.8 * spotAmt) * keyReach(p) * shadowAt(p + n * 0.01);
    // occlusion from the neighbours + a little self-shadow on the underside
    float occ = 1.0;
    for (int j = 0; j < 20; j++){
        if (j >= N) break; if (j == i) continue;
        vec3 dv = bob[j] - p; float l = length(dv);
        occ *= 1.0 - 0.5 * max(dot(n, dv / l), 0.0) * smoothstep(0.6, 0.0, l - R);
    }
    col *= occ * (0.75 + 0.25 * smoothstep(-0.8, 0.3, n.y));
    return col;
}

// closest approach between a ray and a segment: returns (distance, t along ray)
vec2 raySeg(vec3 ro, vec3 rd, vec3 a, vec3 b){
    vec3 ba = b - a, w0 = ro - a;
    float bb = dot(rd, ba), cc = dot(ba, ba), dd = dot(rd, w0), ee = dot(ba, w0);
    float den = max(cc - bb * bb, 1e-6);
    float s = clamp((ee - bb * dd) / den, 0.0, 1.0);
    vec3 q = a + ba * s;
    float t = dot(q - ro, rd);
    return vec2(length(ro + rd * t - q), t);
}

vec3 hueRotate(vec3 c, float a){
    const vec3 k = vec3(0.57735);
    float ca = cos(a);
    return c * ca + cross(k, c) * sin(a) + k * dot(k, c) * (1.0 - ca);
}

void main(){
    vec2 res = dimensions;
    vec2 pc = vec2(coord.x, res.y - coord.y);                 // up-is-up
    vec2 uv = (2.0 * pc - res) / res.y;

    N = int(clamp(count, 4.0, 20.0));
    for (int i = 0; i < 20; i++){
        if (i >= N) break;
        float x = (-0.5 + float(i) / float(N - 1)) * ROW;
        piv[i] = vec3(x, H, 0.0);
        bob[i] = piv[i] + lengths[i] * vec3(0.0, -cos(angles[i]), sin(angles[i]));
    }
    blendLights(time);
    // mids warm the key a touch
    keyCol *= mix(vec3(0.92, 0.96, 1.05), vec3(1.06, 0.98, 0.9), warmth);

    // ---------------- camera: three framings, slow eased dollies between them ----------------
    float ct = time / 34.0;
    float cf = fract(ct);
    int c0 = int(mod(floor(ct), 3.0)), c1 = int(mod(floor(ct) + 1.0, 3.0));
    vec3 P[3]; vec3 Tg[3];
    P[0] = vec3(-2.05, 0.50, 2.65); Tg[0] = vec3(0.30, 0.92, 0.0);   // low 3/4
    P[1] = vec3(-4.2, 0.75, 1.35); Tg[1] = vec3(0.60, 0.95, 0.0);   // down the row: the snake reads
    P[2] = vec3(0.5, 0.95, 3.3);   Tg[2] = vec3(0.00, 0.90, 0.0);   // front, wide
    float mb = smoothstep(0.70, 1.0, cf);
    mb = mb * mb * (3.0 - 2.0 * mb);
    vec3 ro = mix(P[c0], P[c1], mb);
    vec3 ta = mix(Tg[c0], Tg[c1], mb);
    // slow dolly within each hold + music orbit
    vec3 fw0 = normalize(ta - ro);
    ro += fw0 * 0.35 * sin(cf * PI);
    float oa = (orbit + 0.08 * sin(time * 0.05)) * cameraDrift;
    vec3 rel = ro - ta;
    rel.xz = mat2(cos(oa), -sin(oa), sin(oa), cos(oa)) * rel.xz;
    ro = ta + rel;
    vec3 fw = normalize(ta - ro);
    vec3 rt = normalize(cross(fw, vec3(0.0, 1.0, 0.0)));
    vec3 up = cross(rt, fw);
    float fl = 1.9;
    vec3 rd = normalize(uv.x * rt + uv.y * up + fl * fw);
    float focus = length(ta - ro);
    float px = 1.0 / (res.y * fl);                            // world size of a pixel at unit distance (x2 for uv)

    // ---------------- background: floor or gallery dark ----------------
    vec3 col;
    float tScene = 1e9;
    if (rd.y < 0.0){
        float tf = -ro.y / rd.y;
        tScene = tf;
        vec3 p = ro + rd * tf;
        col = floorShade(p, rd, true);
        col = mix(col, horizonCol(), max(smoothstep(2.2, 7.5, tf), smoothstep(-0.10, 0.0, rd.y)));    // seamless studio cove: no horizon line
    } else {
        col = mix(horizonCol(), wallCol * 0.45, smoothstep(0.0, 0.6, rd.y));
    }
    // a faint glow in the air behind the sculpture where the key spills (both above and below the horizon: no step)
    col += keyCol * 0.007 * keyLight * exp(-dot(rd.xy - vec2(0.0, 0.15), rd.xy - vec2(0.0, 0.15)) * 4.0) * (1.0 - spotAmt * 0.8);

    // ---------------- beam ----------------
    {
        vec3 bmin = vec3(-ROW * 0.5 - 0.35, H, -0.07), bmax = vec3(ROW * 0.5 + 0.35, H + 0.11, 0.07);
        vec3 inv = 1.0 / rd;
        vec3 t0 = (bmin - ro) * inv, t1 = (bmax - ro) * inv;
        vec3 tmn = min(t0, t1), tmx = max(t0, t1);
        float tn = max(max(tmn.x, tmn.y), tmn.z), tx = min(min(tmx.x, tmx.y), tmx.z);
        if (tn < tx && tn > 0.0 && tn < tScene){
            vec3 p = ro + rd * tn;
            vec3 n = vec3(0.0);
            if (tn == tmn.x) n.x = -sign(rd.x); else if (tn == tmn.y) n.y = -sign(rd.y); else n.z = -sign(rd.z);
            vec3 bc = vec3(0.025, 0.022, 0.02) * (ambCol * 8.0 + keyCol * max(dot(n, keyDir), 0.0) * 0.25);
            bc += keyCol * 0.08 * pow(max(dot(reflect(rd, n), keyDir), 0.0), 40.0);
            col = bc; tScene = tn;
        }
    }

    // ---------------- wires ----------------
    for (int i = 0; i < 20; i++){
        if (i >= N) break;
        vec2 ws = raySeg(ro, rd, piv[i], bob[i]);
        if (ws.y <= 0.0 || ws.y > tScene) continue;
        float coc = depthOfField * abs(ws.y - focus) / max(ws.y, 0.1) * 0.035;
        float w = 2.0 * px * ws.y + coc;
        float a = smoothstep(w, 0.0, ws.x) * clamp(0.0035 / w, 0.0, 1.0);
        vec3 wc = ambCol * 1.5 + keyCol * 0.012 * keyLight + stripCol * 0.02;
        // a wire is dark steel that only catches a glint where it crosses the key's reflection
        vec3 wd = normalize(bob[i] - piv[i]);
        // a cylinder shows a highlight only where the half-vector is perpendicular to it
        float glint = pow(1.0 - abs(dot(wd, normalize(keyDir - rd))), 24.0);
        col = mix(col, wc + keyCol * 0.09 * keyLight * glint, a * 0.85);
    }

    // ---------------- spheres: two nearest, soft (depth-of-field) coverage ----------------
    float tA = 1e9, tB = 1e9, cA = 0.0, cB = 0.0; int iA = -1, iB = -1;
    float glow = 0.0;
    for (int i = 0; i < 20; i++){
        if (i >= N) break;
        vec3 oc = bob[i] - ro; float tc = dot(oc, rd);
        if (tc <= 0.0) continue;
        float d = length(ro + rd * tc - bob[i]);
        float coc = depthOfField * abs(tc - focus) / max(tc, 0.1) * 0.035;
        float w = 1.5 * px * tc + coc;
        float c = smoothstep(R + w, R - w, d);
        glow += exp(-max(d - R, 0.0) / (0.06 + coc)) * 0.5 / (1.0 + tc * 0.3);
        if (c < 0.002 || tc > tScene) continue;
        if (tc < tA){ tB = tA; cB = cA; iB = iA; tA = tc; cA = c; iA = i; }
        else if (tc < tB){ tB = tc; cB = c; iB = i; }
    }
    for (int k = 0; k < 2; k++){
        int i = k == 0 ? iB : iA;
        float c = k == 0 ? cB : cA;
        if (i < 0) continue;
        vec3 oc = ro - bob[i]; float b = dot(oc, rd); float h = b * b - dot(oc, oc) + R * R;
        vec3 p, n;
        if (h > 0.0){ p = ro + rd * (-b - sqrt(h)); n = normalize(p - bob[i]); }
        else { float tc = -b; vec3 q = ro + rd * tc; n = normalize(q - bob[i]); p = bob[i] + n * R; }
        col = mix(col, sphereShade(i, p, n, rd), c);
    }
    col += (keyCol * 0.010 + rimCol * 0.004) * glow * (0.6 + 1.2 * sparkle) * keyLight;

    // ---------------- haze: the key light in the air, with dust ----------------
    {
        float tEnd = min(tScene, 12.0);
        float acc = 0.0; float dust = 0.0;
        float jit = hash13(vec3(pc, fract(time) * 7.0));
        for (int s = 0; s < 12; s++){
            float t = tEnd * (float(s) + jit) / 12.0;
            vec3 p = ro + rd * t;
            float dens = keyReach(p) * smoothstep(3.6, 0.5, p.y) * smoothstep(-0.1, 0.4, p.y);
            dens *= mix(1.0, smoothstep(2.2, 0.8, length(p.xz - vec2(0.0, 0.2))), spotAmt);
            acc += dens;
        }
        acc *= tEnd / 12.0;
        float phase = 0.5 + 0.8 * pow(max(dot(rd, keyDir) * -1.0 + 1.0, 0.0) * 0.5, 2.0);
        col += keyCol * acc * haze * mix(0.0028, 0.011, spotAmt) * phase * (1.0 + 0.6 * smoothstep(0.85, 1.0, align));
        // dust motes: a sparse 3D grid sampled on two depth slices
        for (int s = 0; s < 2; s++){
            float t = focus * (0.6 + 0.8 * float(s));
            vec3 p = ro + rd * t;
            vec3 g = p * 9.0;
            vec3 id = floor(g); vec3 f = fract(g) - 0.5;
            float h = hash13(id);
            if (h > 0.95 && p.y > 0.15){
                vec3 o = vec3(hash13(id + 1.7), hash13(id + 3.1), hash13(id + 5.3)) - 0.5;
                float dd = length(f - o * 0.6);
                float tw = 0.5 + 0.5 * sin(time * (0.7 + 2.0 * h) + h * 50.0);
                dust += exp(-dd * dd * 900.0) * tw * keyReach(p) * (0.3 + 1.7 * sparkle);
            }
        }
        col += keyCol * dust * haze * 0.08;
    }

    // re-alignment payoff: the rim light swells as the row lines up
    col *= 1.0 + 0.25 * smoothstep(0.88, 1.0, align);

    // ---------------- grade ----------------
    col = hueRotate(col, hueShift * 6.2831853);
    col *= brightness * 1.8;
    col = col * (2.51 * col + 0.03) / (col * (2.43 * col + 0.59) + 0.14);      // ACES
    float vig = smoothstep(1.9, 0.5, length(uv * vec2(0.8, 1.0)));
    col *= mix(0.6, 1.0, vig);
    col += (hash13(vec3(pc, fract(time * 13.0) * 100.0)) - 0.5) / 255.0 * 1.5;
    outputColor = vec4(clamp(col, 0.0, 1.0), 1.0);
}
