#version 150

// Ported from "Trailing the Twinkling Tunnelwisp" by @byt3_m3chanic / mrange (CC0).
// A single-loop gyroid raymarcher: a translucent marble cave tunnel with a
// water-mirror reflection and a pulsing "tunnelwisp" glow.

uniform sampler2D tex;
uniform vec2 dimensions;
uniform float time;
uniform float speed;        // traversal + pulse speed
uniform float twist;        // how much the cave walls corkscrew with depth
uniform float offset;       // manual depth shift along the tunnel (in/out)
uniform float steps;        // raymarch iterations (depth / detail vs cost)
uniform float detailA;      // gyroid scale 1 (coarse wall structure)
uniform float detailB;      // gyroid scale 2 (fine wall structure)
uniform float density;      // wall thickness divisor (lower = denser, brighter)
uniform float translucency; // march epsilon (higher = more see-through, hazier)
uniform float reflection;   // brightness of the mirrored "water" half
uniform float poolHeight;   // vertical position of the water surface
uniform float colorSpeed;   // how fast hue cycles with depth
uniform float hueShift;     // overall hue phase offset
uniform float wisp;         // brightness of the pulsing tunnelwisp
uniform float brightness;   // exposure

// Live audio: 256-bin magnitude spectrum, normalized 0-1 (all 0 when silent).
uniform float audio[256];
uniform float audioAmount;  // scale of the spectrum->wall effect (0 = off)

in vec2 coord;
out vec4 outputColor;

// Gyroid distance field, adapted from Paul Karlik's "Gyroid Travel".
float g(vec4 p, float s) {
  return abs(dot(sin(p *= s), cos(p.zxwy)) - 1.) / s;
}

// Map a cross-section direction to a spectrum band: the angle around the tunnel
// picks a frequency bin, so bass lights one arc of the cave and treble another.
// A couple of neighbouring bins are averaged to reduce single-bin flicker.
float audioBand(vec2 dir) {
  float a = atan(dir.y, dir.x) / 6.2831853 + 0.5; // [0,1] around the tunnel
  int idx = clamp(int(a * 255.), 0, 254);
  return 0.5 * (audio[idx] + audio[idx + 1]);
}

void main() {
  // All locals explicitly zero-initialized: the original relies on driver
  // zero-init of uninitialized locals, which is undefined behavior.
  float i = 0., d = 0., z = 0., s = 0., b = 0., T = time * speed;
  vec4 o = vec4(0.), q = vec4(0.), p = vec4(0.), U = vec4(2, 1, 0, 3);
  vec2 r = dimensions.xy;
  vec2 C = coord.xy;

  for (
    // Step through the scene
    ; ++i < steps
    // Advance along the ray; epsilon makes the cave walls translucent
    ; z += d + translucency
    // Ray direction, scaled by distance
    , q = vec4(normalize(vec3(C - .5 * r, r.y)) * z, .2)
    // Traverse through the cave (offset shifts position in/out)
    , q.z += T / 3E1 + offset
    // Save sign before mirroring (pool surface at y = -poolHeight)
    , s = q.y + poolHeight
    // Water reflection: mirror across the axis
    , q.y = abs(s)
    , p = q
    , p.y -= .11
    // Twist cave walls based on depth (twist knob scales the rate)
    , p.xy *= mat2(cos(11. * U.zywz - 2. * twist * p.z))
    , p.y -= .2
    // Combine gyroid fields at two scales for detail
    , d = abs(g(p, detailA) - g(p, detailB)) / density
    // Sample the spectrum for this wall segment (p.xy is still the cross-section)
    , b = audioBand(p.xy)
    // Base glow color varies with distance from center
    , p = 1. + cos(.7 * U + colorSpeed * q.z + hueShift)
  )
    // Accumulate glow — brighter/sharper above the mirror axis, and bloomed
    // where this arc's frequency band is loud (audioAmount scales the effect)
    o += (s > 0. ? 1. : reflection) * p.w * p * (1. + audioAmount * b)
         / max(s > 0. ? d : d * d * d, 5E-4);

  // Pulsing glow for the "tunnelwisp"
  o += wisp * (1.4 + sin(T) * sin(1.7 * T) * sin(2.3 * T))
       * 1E3 * U / length(q.xy);

  // Soft tone mapping (brightness scales exposure)
  o = tanh(o / (1E5 / brightness));

  outputColor = vec4(o.rgb, 1.0);
}
