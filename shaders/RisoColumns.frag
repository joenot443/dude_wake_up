#version 150

uniform sampler2D tex;
uniform vec2 dimensions;
uniform float time;          // speed-scaled accumulated phase (see shader)

uniform float columns;       // number of tile columns (square-ish tiles)
uniform float colorFolds;    // color wraps around the circle = number of hard seams ("hands")
uniform float grainAmount;   // film-grain strength
uniform float colorShift;    // 0..1, hue rotation of the whole field

in vec2 coord;
out vec4 outputColor;

float hash21(vec2 p){
  p = fract(p*vec2(123.34, 345.45));
  p += dot(p, p+34.345);
  return fract(p.x*p.y);
}

// Conic palette sampled directly from the reference: peach -> orange -> red ->
// coral -> rose -> mauve -> purple -> periwinkle -> blue -> deep indigo, then a
// HARD wrap back to peach. That single wrap is the only hard edge -- it becomes
// the tight, crisp rotating "clock hand". Everything else is a smooth sweep.
vec3 conic(float u){
  u = fract(u);
  const int N = 10;
  float pos[10] = float[10](0.00, 0.12, 0.26, 0.36, 0.46, 0.56, 0.66, 0.78, 0.88, 1.00);
  vec3  cols[10] = vec3[10](
    vec3(0.90, 0.69, 0.55),   // peach
    vec3(0.91, 0.58, 0.36),   // orange
    vec3(0.90, 0.43, 0.25),   // red-orange
    vec3(0.90, 0.37, 0.26),   // red
    vec3(0.88, 0.39, 0.36),   // coral
    vec3(0.79, 0.41, 0.47),   // rose
    vec3(0.64, 0.42, 0.61),   // mauve
    vec3(0.50, 0.45, 0.73),   // periwinkle
    vec3(0.37, 0.37, 0.74),   // blue
    vec3(0.39, 0.19, 0.56));  // deep indigo (pre-seam)
  vec3 c = cols[0];
  for(int i = 0; i < N-1; i++){
    c = mix(c, cols[i+1], smoothstep(pos[i], pos[i+1], u));
  }
  return c;
}

vec3 hueRotate(vec3 c, float a){
  const vec3 k = vec3(0.57735026919);
  float cs = cos(a);
  return c*cs + cross(k, c)*sin(a) + k*dot(k, c)*(1.0 - cs);
}

void main(){
  vec2 uvn = coord / dimensions;            // 0..1
  // square-ish tiles regardless of output aspect
  float cols = max(1.0, columns);
  float rows = max(1.0, floor(cols * dimensions.y / dimensions.x + 0.5));
  vec2 t = uvn * vec2(cols, rows);
  vec2 cell = floor(t);
  vec2 f = fract(t);

  float h  = hash21(cell);
  float h2 = hash21(cell + 11.7);

  // tile-local coord centered at the square's middle (-0.5..0.5, isotropic)
  vec2 p = f - 0.5;

  // each tile's seam rotates around its center at a per-tile rate & direction
  float dir   = (h2 < 0.5) ? -1.0 : 1.0;
  float speed = mix(0.12, 0.70, h) * dir;
  float ang   = h*6.2831853 + speed*time;

  // conic gradient: color by angle around center, relative to the rotating hand.
  // The fract() wrap of the palette puts the single hard edge exactly on `ang`,
  // pointing center -> edge (so the "hand" is naturally half a diameter).
  float a   = atan(p.y, p.x) - ang;
  float rel = a / 6.2831853;                 // in turns; seam sits on the hand
  float u   = rel * max(1.0, colorFolds);    // tight, crisp seam (no jitter)

  vec3 col = conic(u);

  // hue rotate the whole field
  if(colorShift > 0.0001) col = hueRotate(col, colorShift * 6.2831853);

  // gentle radial falloff toward tile edges -> soft seams / a little depth
  float vign = smoothstep(0.95, 0.15, length(p));
  col *= mix(0.82, 1.0, vign);

  // heavy animated film grain in pixel space
  vec2 gp = coord + vec2(time*57.0, time*113.0);
  float grain = hash21(floor(gp)) - 0.5;
  col += grain * grainAmount;

  col = clamp(col, 0.0, 1.0);
  outputColor = vec4(col, 1.0);
}
