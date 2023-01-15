#version 150

uniform sampler2D tex;
uniform vec2 dimensions;
uniform float time;
in vec2 coord;
out vec4 outputColor;

#define LOOP_DURATION 5.
#define MOVE_COUNT 6.
#define TIME_OFFSET .3

bool lightingPass;

// axisX, axisY, axisZ, turns
vec4 moves[6] =
    vec4[6](vec4(1, 0, 0, 2.), vec4(0, 1, 0, -1.), vec4(0, -1, 0, -3.),
            vec4(0, 0, -1, 2.), vec4(0, -1, 0, -1.), vec4(0, 1, 0, -3.));

// Quaternions
// https://github.com/mattatz/ShibuyaCrowd/blob/master/source/shaders/common/quaternion.glsl

#define QUATERNION_IDENTITY vec4(0, 0, 0, 1)

#define PI 3.1415926

// Quaternion multiplication
// http://mathworld.wolfram.com/Quaternion.html
vec4 qmul(vec4 q1, vec4 q2) {
  return vec4(q2.xyz * q1.w + q1.xyz * q2.w + cross(q1.xyz, q2.xyz),
              q1.w * q2.w - dot(q1.xyz, q2.xyz));
}

// Vector rotation with a quaternion
// http://mathworld.wolfram.com/Quaternion.html
vec3 rotate_vector(vec3 v, vec4 r) {
  vec4 r_c = r * vec4(-1, -1, -1, 1);
  return qmul(r, qmul(vec4(v, 0), r_c)).xyz;
}

// A given angle of rotation about a given axis
vec4 rotate_angle_axis(float angle, vec3 axis) {
  float sn = sin(angle * 0.5);
  float cs = cos(angle * 0.5);
  return vec4(axis * sn, cs);
}

vec4 q_conj(vec4 q) { return vec4(-q.x, -q.y, -q.z, q.w); }

vec4 q_slerp(vec4 a, vec4 b, float t) {
  // if either input is zero, return the other.
  if (length(a) == 0.0) {
    if (length(b) == 0.0) {
      return QUATERNION_IDENTITY;
    }
    return b;
  } else if (length(b) == 0.0) {
    return a;
  }

  float cosHalfAngle = a.w * b.w + dot(a.xyz, b.xyz);

  if (cosHalfAngle >= 1.0 || cosHalfAngle <= -1.0) {
    return a;
  } else if (cosHalfAngle < 0.0) {
    b.xyz = -b.xyz;
    b.w = -b.w;
    cosHalfAngle = -cosHalfAngle;
  }

  float blendA;
  float blendB;
  if (cosHalfAngle < 0.99) {
    // do proper slerp for big angles
    float halfAngle = acos(cosHalfAngle);
    float sinHalfAngle = sin(halfAngle);
    float oneOverSinHalfAngle = 1.0 / sinHalfAngle;
    blendA = sin(halfAngle * (1.0 - t)) * oneOverSinHalfAngle;
    blendB = sin(halfAngle * t) * oneOverSinHalfAngle;
  } else {
    // do lerp if angle is really small.
    blendA = 1.0 - t;
    blendB = t;
  }

  vec4 result =
      vec4(blendA * a.xyz + blendB * b.xyz, blendA * a.w + blendB * b.w);
  if (length(result) > 0.0) {
    return normalize(result);
  }
  return QUATERNION_IDENTITY;
}

//========================================================
// Utils
//========================================================

// HG_SDF

void pR(inout vec2 p, float a) { p = cos(a) * p + sin(a) * vec2(p.y, -p.x); }

float vmin(vec3 v) { return min(min(v.x, v.y), v.z); }

float vmax(vec3 v) { return max(max(v.x, v.y), v.z); }

float fBox(vec3 p, vec3 b) {
  vec3 d = abs(p) - b;
  return length(max(d, vec3(0))) + vmax(min(d, vec3(0)));
}

float smin(float a, float b, float k) {
  float f = clamp(0.5 + 0.5 * ((a - b) / k), 0., 1.);
  return (1. - f) * a + f * b - f * (1. - f) * k;
}

float smax(float a, float b, float k) { return -smin(-a, -b, k); }

// Easings

float range(float vmin, float vmax, float value) {
  return clamp((value - vmin) / (vmax - vmin), 0., 1.);
}

float almostIdentity(float x) { return x * x * (2.0 - x); }

float circularOut(float t) { return sqrt((2.0 - t) * t); }

// Spectrum palette, iq https://www.shadertoy.com/view/ll2GD3

vec3 pal(in float t, in vec3 a, in vec3 b, in vec3 c, in vec3 d) {
  return a + b * cos(6.28318 * (c * t + d));
}

vec3 spectrum(float n) {
  return pal(n, vec3(0.5, 0.5, 0.5), vec3(0.5, 0.5, 0.5), vec3(1.0, 1.0, 1.0),
             vec3(0.0, 0.33, 0.67));
}

// rotate on axis, blackle
vec3 erot(vec3 p, vec3 ax, float ro) {
  return mix(dot(ax, p) * ax, p, cos(ro)) + sin(ro) * cross(ax, p);
}

//========================================================
// Animation
//========================================================

// see common tab for a list of moves (rotations)

void applyMomentum(inout vec4 q, float time, int i, vec4 move) {

  float turns = move.w;
  vec3 axis = move.xyz;

  float duration = abs(turns);
  float rotation = PI / 2. * turns * .75;

  float start = float(i + 1);
  float t = time * MOVE_COUNT;
  float ramp = range(start, start + duration, t);
  float angle = circularOut(ramp) * rotation;
  vec4 q2 = rotate_angle_axis(angle, axis);
  q = qmul(q, q2);
}

void applyMove(inout vec3 p, int i, vec4 move) {

  float turns = move.w;
  vec3 axis = move.xyz;

  float rotation = PI / 2. * turns;

  float start = float(i);
  float t = time * MOVE_COUNT;
  float ramp = range(start, start + 1., t);
  ramp = pow(almostIdentity(ramp), 2.5);
  float angle = ramp * rotation;

  bool animSide = vmax(p * -axis) > 0.;
  if (animSide) {
    angle = 0.;
  }

  p = erot(p, axis, angle);
}

vec4 momentum(float time) {
  vec4 q = QUATERNION_IDENTITY;
  applyMomentum(q, time, 5, moves[5]);
  applyMomentum(q, time, 4, moves[4]);
  applyMomentum(q, time, 3, moves[3]);
  applyMomentum(q, time, 2, moves[2]);
  applyMomentum(q, time, 1, moves[1]);
  applyMomentum(q, time, 0, moves[0]);
  return q;
}

vec4 momentumLoop(float time) {
  vec4 q;

  // end state
  q = momentum(3.);
  q = q_conj(q);
  q = q_slerp(QUATERNION_IDENTITY, q, time);

  // next loop
  q = qmul(momentum(time + 1.), q);

  // current loop
  q = qmul(momentum(time), q);

  return q;
}

//========================================================
// Modelling
//========================================================

vec4 mapBox(vec3 p) {

  // shuffle blocks
  pR(p.xy, step(0., -p.z) * PI / -2.);
  pR(p.xz, step(0., p.y) * PI);
  pR(p.yz, step(0., -p.x) * PI * 1.5);

  // face colors
  vec3 face = step(vec3(vmax(abs(p))), abs(p)) * sign(p);
  float faceIndex =
      max(vmax(face * vec3(0, 1, 2)), vmax(face * -vec3(3, 4, 5)));
  vec3 col = spectrum(faceIndex / 6. + .1 + .5);

  // offset sphere shell
  float thick = .033;
  float d = length(p + vec3(.1, .02, .05)) - .4;
  d = max(d, -d - thick);

  // grooves
  vec3 ap = abs(p);
  float l = sqrt(sqrt(1.) / 3.);
  vec3 plane = cross(abs(face), normalize(vec3(1)));
  float groove = max(-dot(ap.yzx, plane), dot(ap.zxy, plane));
  d = smax(d, -abs(groove), .01);

  float gap = .005;

  // block edge
  float r = .05;
  float cut = -fBox(abs(p) - (1. + r + gap), vec3(1.)) + r;
  d = smax(d, -cut, thick / 2.);

  // adjacent block edge bounding
  float opp = vmin(abs(p)) + gap;
  opp = max(opp, length(p) - 1.);
  if (opp < d) {
    return vec4(opp, vec3(-1));
  }

  return vec4(d, col * .4);
}

vec4 map(vec3 p) {
  // p.z *= -1.;
  pR(p.xz, time * PI * 2.);
  // pR(p.yz, time * PI * -2.);
  // pR(p.xy, PI);

  vec4 q = momentumLoop(time);
  p = rotate_vector(p, q);

  applyMove(p, 5, moves[5]);
  applyMove(p, 4, moves[4]);
  applyMove(p, 3, moves[3]);
  applyMove(p, 2, moves[2]);
  applyMove(p, 1, moves[1]);
  applyMove(p, 0, moves[0]);

  return mapBox(p);
}

//========================================================
// Rendering
//========================================================

mat3 calcLookAtMatrix(in vec3 ro, in vec3 ta, in float roll) {
  vec3 ww = normalize(ta - ro);
  vec3 uu = normalize(cross(ww, vec3(sin(roll), cos(roll), 0.0)));
  vec3 vv = normalize(cross(uu, ww));
  return mat3(uu, vv, ww);
}

// https://iquilezles.org/articles/normalsSDF
vec3 calcNormal(vec3 p) {
  const float h = 0.001;
  vec3 n = vec3(0.0);
  for (int i = 0; i < 4; i++) {
    vec3 e = 0.5773 *
             (2.0 * vec3((((i + 3) >> 1) & 1), ((i >> 1) & 1), (i & 1)) - 1.0);
    n += e * map(p + e * h).x;
  }
  return normalize(n);
}

// origin sphere intersection
// returns entry and exit distances from ray origin
vec2 iSphere(in vec3 ro, in vec3 rd, float r) {
  vec3 oc = ro;
  float b = dot(oc, rd);
  float c = dot(oc, oc) - r * r;
  float h = b * b - c;
  if (h < 0.0)
    return vec2(-1.0);
  h = sqrt(h);
  return vec2(-b - h, -b + h);
}

// https://www.shadertoy.com/view/lsKcDD
float softshadow(in vec3 ro, in vec3 rd, in float mint, in float tmax) {
  float res = 1.0;

  // iq optimisation, stop looking for occluders when we
  // exit the bounding sphere for the model
  vec2 bound = iSphere(ro, rd, .55);
  tmax = min(tmax, bound.y);

  float t = mint;
  float ph = 1e10;

  for (int i = 0; i < 100; i++) {
    vec4 hit = map(ro + rd * t);
    float h = hit.x;
    if (hit.y > 0.) { // don't create shadows from bounding objects
      res = min(res, 10.0 * h / t);
    }
    t += h;
    if (res < 0.0001 || t > tmax)
      break;
  }

  return clamp(res, 0.0, 1.0);
}

vec3 render(vec2 p) {

  vec3 col = vec3(.02, .01, .025);

  // raymarch

  vec3 camPos = vec3(0, 0, 2.);
  mat3 camMat = calcLookAtMatrix(camPos, vec3(0, 0, -1), 0.);
  vec3 rd = normalize(camMat * vec3(p.xy, 2.8));
  vec3 pos = camPos;

  vec2 bound = iSphere(pos, rd, .55);
  if (bound.x < 0.) {
    return col;
  }

  lightingPass = false;
  float rayLength = bound.x;
  float dist = 0.;
  bool background = true;
  vec4 res;

  for (int i = 0; i < 200; i++) {
    rayLength += dist;
    pos = camPos + rd * rayLength;
    res = map(pos);
    dist = res.x;

    if (abs(dist) < .001) {
      background = false;
      break;
    }

    if (rayLength > bound.y) {
      break;
    }
  }

  // shading
  // https://www.shadertoy.com/view/Xds3zN

  lightingPass = true;

  if (!background) {

    col = res.yzw;
    vec3 nor = calcNormal(pos);
    vec3 lig = normalize(vec3(-.33, .3, .25));
    vec3 lba = normalize(vec3(.5, -1., -.5));
    vec3 hal = normalize(lig - rd);
    float amb = sqrt(clamp(0.5 + 0.5 * nor.y, 0.0, 1.0));
    float dif = clamp(dot(nor, lig), 0.0, 1.0);
    float bac = clamp(dot(nor, lba), 0.0, 1.0) * clamp(1.0 - pos.y, 0.0, 1.0);
    float fre = pow(clamp(1.0 + dot(nor, rd), 0.0, 1.0), 2.0);

    // iq optimisation, skip shadows when we're facing away
    // from the light
    if (dif > .001)
      dif *= softshadow(pos, lig, 0.001, .9);

    float occ = 1.;

    float spe = pow(clamp(dot(nor, hal), 0.0, 1.0), 16.0) * dif *
                (0.04 + 0.96 * pow(clamp(1.0 + dot(hal, rd), 0.0, 1.0), 5.0));

    vec3 lin = vec3(0.0);
    lin += 2.80 * dif * vec3(1.30, 1.00, 0.70);
    lin += 0.55 * amb * vec3(0.40, 0.60, 1.15) * occ;
    lin += 1.55 * bac * vec3(0.25, 0.25, 0.25) * occ * vec3(2, 0, 1);
    lin += 0.25 * fre * vec3(1.00, 1.00, 1.00) * occ;

    col = col * lin;
    col += 5.00 * spe * vec3(1.10, 0.90, 0.70);
  }

  return col;
}

float vmul(vec2 v) { return v.x * v.y; }

void main() {

  float mTime = (time + TIME_OFFSET) / LOOP_DURATION;

  mTime = .01;
  float nTime = mTime;

  vec2 o = vec2(0);
  vec3 col = vec3(0);

  // // AA and motion blur from iq https://www.shadertoy.com/view/3lsSzf
  // #ifdef AA
  //   for (int m = 0; m < AA; m++)
  //     for (int n = 0; n < AA; n++) {
  //       // pixel coordinates
  //       o = vec2(float(m), float(n)) / float(AA) - 0.5;
  //       // time coordinate (motion blurred, shutter=0.5)
  //       float d = 0.5 * vmul(sin(mod(coord.xy * vec2(147, 131), vec2(PI
  //       * 2.)))); nTime = mTime -
  //               0.1 * (1.0 / 24.0) * (float(m * AA + n) + d) / float(AA * AA
  //               - 1);
  // #endif

  nTime = mod(nTime, 1.);
  vec2 p = (-dimensions.xy + 2. * (coord + o)) / dimensions.y;
  col += render(p);

  // #ifdef AA
  //     }
  //   col /= float(AA * AA);
  // #endif

  col = pow(col, vec3(0.4545));

  outputColor = vec4(col, 1.);
}
