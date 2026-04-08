#version 150

uniform sampler2D tex;
in vec2 coord;
uniform float scale;
uniform float sides;
uniform float rotation;
uniform float shift;
uniform vec2 dimensions;
out vec4 outputColor;

const float PI = 3.14159265;

vec2 rotate2d(vec2 p, float a) {
  float c = cos(a), s = sin(a);
  return vec2(c * p.x - s * p.y, s * p.x + c * p.y);
}

// Reflect point across a line through the origin with given normal
vec2 reflectLine(vec2 p, vec2 n) {
  return p - 2.0 * dot(p, n) * n;
}

void main()
{
  vec2 uv = (coord - 0.5) * scale;
  uv = rotate2d(uv, rotation);
  uv += shift;

  // Build the fundamental triangle based on the "sides" parameter.
  // sides=3 -> 60-60-60 equilateral (wallpaper group p6m)
  // sides=4 -> 45-45-90 isosceles right (wallpaper group p4m)
  // sides=6 -> 30-60-90 (wallpaper group p6m)
  // Other values are clamped/interpolated to nearest valid triangle.

  // Angle at the apex of the fundamental triangle
  float alpha = PI / sides;
  // The two base angles of the Schwarz triangle: PI/2 and PI/q
  // For a valid Euclidean triangle: 1/2 + 1q + 1/sides = 1, so q = 1/(0.5 - 1/sides)
  // sides=3: q=6 (but we use equilateral: alpha=beta=gamma=PI/3)
  // sides=4: q=4, triangle is 45-45-90
  // sides=6: q=3, triangle is 30-60-90

  // Define the three vertices of the fundamental triangle
  // Vertex A at origin, vertex B along x-axis, vertex C at angle alpha
  float triBase = 1.0;
  vec2 vA = vec2(0.0);
  vec2 vB = vec2(triBase, 0.0);
  vec2 vC = vec2(triBase * cos(alpha), triBase * sin(alpha));

  // Compute inward-pointing normals for the three edges
  // Edge AB: from A to B (along x-axis), normal points up
  vec2 eAB = normalize(vB - vA);
  vec2 nAB = vec2(-eAB.y, eAB.x); // (0, 1)

  // Edge BC: from B to C
  vec2 eBC = normalize(vC - vB);
  vec2 nBC = vec2(-eBC.y, eBC.x);

  // Edge CA: from C to A
  vec2 eCA = normalize(vA - vC);
  vec2 nCA = vec2(-eCA.y, eCA.x);

  // Iteratively reflect the point into the fundamental domain.
  // Each reflection is across one of the three mirror edges.
  // We iterate until the point is inside the triangle (all dot products >= 0).
  for (int i = 0; i < 64; i++) {
    bool reflected = false;

    // Check edge AB (passes through vA, normal nAB)
    float dAB = dot(uv - vA, nAB);
    if (dAB < 0.0) {
      uv = uv - 2.0 * dAB * nAB;
      reflected = true;
    }

    // Check edge BC (passes through vB, normal nBC)
    float dBC = dot(uv - vB, nBC);
    if (dBC < 0.0) {
      uv = uv - 2.0 * dBC * nBC;
      reflected = true;
    }

    // Check edge CA (passes through vC, normal nCA)
    float dCA = dot(uv - vC, nCA);
    if (dCA < 0.0) {
      uv = uv - 2.0 * dCA * nCA;
      reflected = true;
    }

    if (!reflected) break;
  }

  // Map the fundamental domain to texture coordinates [0,1]
  // Project onto the triangle's local axes
  vec2 texUV = vec2(
    uv.x / triBase,
    uv.y / (triBase * sin(alpha))
  );
  texUV = clamp(texUV, 0.0, 1.0);

  outputColor = texture(tex, texUV);
}
