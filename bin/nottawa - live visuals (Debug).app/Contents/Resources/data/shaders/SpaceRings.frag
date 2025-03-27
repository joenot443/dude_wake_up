#version 150

#define p0 0.5, 0.5, 0.5,  0.5, 0.5, 0.5,  1.0, 1.0, 1.0,  0.0, 0.33, 0.67

uniform sampler2D tex;
uniform vec2 dimensions;
uniform float time;
uniform float particleSize;
uniform float ringCount;
uniform float colorVariation;
in vec2 coord;
out vec4 outputColor;

const float numParticles = 25.;
const float offsetMult = 30.;
const float tau = 6.23813;

vec3 palette( in float t, in float a0, in float a1, in float a2, in float b0, in float b1, in float b2,
              in float c0, in float c1, in float c2, in float d0, in float d1, in float d2)
{
    return vec3(a0,a1,a2) + vec3(b0,b1,b2) * cos(tau * (vec3(c0,c1,c2) * t + vec3(d0,d1,d2) * colorVariation));
}

vec3 particleColor(vec2 uv, float radius, float offset, float periodOffset)
{
    vec3 color = palette(.4 + offset / 4., p0);
    uv /= pow(periodOffset, .75) * sin(periodOffset * time) + sin(periodOffset + time);
    vec2 pos = vec2(cos(offset * offsetMult + time + periodOffset),
            sin(offset * offsetMult + time * 5. + periodOffset * tau));
    
    float dist = radius / distance(uv, pos);
    return color * pow(dist, 2.) * 1.75;
}

void main(  )
{
    vec2 uv = (2. * coord - dimensions.xy) / dimensions.y;
    uv *= 3.45;

    outputColor.rgb = vec3(0.);
    
    for (float n = 0.; n <= ringCount; n++)
    {
        for (float i = 0.; i <= numParticles; i++) {
          outputColor.rgb += particleColor(uv, particleSize, i / numParticles, n / 2.);
      }
    }
}
