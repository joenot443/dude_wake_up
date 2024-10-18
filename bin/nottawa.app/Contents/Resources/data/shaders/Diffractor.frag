#version 150

uniform sampler2D tex;
uniform vec2 dimensions;
uniform float time;
uniform float spacing;
uniform float speed;
uniform float scramble;
uniform float scale; // New uniform
uniform int count; // New uniform

in vec2 coord;
out vec4 outputColor;

#define M_PI 3.1415926535897932384626433832795
#define M_TWO_PI (2.0 * M_PI)

// colormap source:
// https://www.shadertoy.com/view/MfjBDV

// https://www.desmos.com/calculator/n4mfhffj1n
float colormap_expf(float x, float v)
{
  if (abs(v) < .0001) v = .0001;
  float p = pow(2., v);
  return (1. - pow(p, -x)) / (1. - 1. / p);
}

vec3 colormap(float x)
{
  float t = .18 * abs(x);
  if (x < 0.)
  {
    x = -x;
    t = -.37 - .14 * x;
  }
  
  // https://www.desmos.com/calculator/sdqk904uu9
  vec3 tone = 8. * vec3(
                        cos(6.283 * t),
                        cos(6.283 * (t - .3333)),
                        cos(6.283 * (t - .6667))
                        );
  
  x = smoothstep(0., 1., x);
  vec3 c = vec3(
                colormap_expf(x, tone.r),
                colormap_expf(x, tone.g),
                colormap_expf(x, tone.b)
                );
  
  c = mix(c, c + vec3(.04, 0, .03), smoothstep(.1, 0., x));
  
  return c;
}

float spow(float a, float b)
{
  return sign(a) * pow(abs(a), b);
}

void main()
{
  vec2 uv = (2. * coord - dimensions.xy) / (min(dimensions.x, dimensions.y) * scale);
  
  float v = 0.;
  float c = count;
  for (int i = -count / 2; i <= count / 2; i++) // Use count uniform
  {
    vec2 center = vec2(spacing * float(i) * scramble*scramble, sin((i / c) * M_TWO_PI) * scramble);
    float dist = distance(uv, center);
    v += sin(100. * dist - speed * time) / (dist + .05);
  }
  
  vec3 col = vec3(colormap(.05 * v));
  
  col = pow(col, vec3(1. / 2.2));
  outputColor = vec4(col, 1);
}
