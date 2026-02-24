#version 150

uniform sampler2D tex;
uniform vec2 dimensions;
uniform float time;
in vec2 coord;
out vec4 outputColor;

uniform float intensity;
uniform float speed;
uniform float glow;

vec3 palette(in float t)
{
    vec3 a = vec3(0.5, 0.5, 0.5);
    vec3 b = vec3(0.5, 0.5, 0.5);
    vec3 c = vec3(2.0, 1.0, 0.0);
    vec3 d = vec3(0.5, 0.2, 0.25);
    return a + b * cos(6.283185 * (c * t + d));
}

void main()
{
    vec2 uv = coord / dimensions;

    vec3 finalColor = vec3(0.0);

    vec2 uvn1 = uv;

    uv -= 0.5;
    uv *= 2.0;
    uv.x *= dimensions.x / dimensions.y;

    vec2 uv0 = uv;

    vec2 uv1 = texture(tex, uvn1).rb;
    vec2 uv2 = texture(tex, uvn1).rg;
    vec2 uv3 = texture(tex, uvn1).gb;

    for (float i = 0.0; i < 5.0; i++) {
        float d1 = (abs(uv1.x) + abs(uv1.y)) * exp(-length(uv0));
        float d2 = (abs(uv2.x) + abs(uv2.y)) * exp(-length(uv0));
        float d3 = (abs(uv3.x) + abs(uv3.y)) * exp(-length(uv0));

        vec3 col1 = intensity * texture(tex, uv1).rgb * palette(length(uv0) + time * speed + i * 0.3);
        vec3 col2 = intensity * texture(tex, uv2).rgb * palette(length(uv0) + time * speed + i * 0.3);
        vec3 col3 = intensity * texture(tex, uv3).rgb * palette(length(uv0) + time * speed + i * 0.3);

        d1 = sin(d1 * 8.0 + time) / 8.0;
        d2 = sin(d2 * 8.0 + time) / 8.0;
        d3 = sin(d3 * 8.0 + time) / 8.0;

        d1 = abs(d1);
        d2 = abs(d2);
        d3 = abs(d3);

        d1 = glow / d1;
        d2 = glow / d2;
        d3 = glow / d3;

        d1 = pow(d1, 1.3);
        d2 = pow(d2, 1.3);
        d3 = pow(d3, 1.3);

        finalColor += col1 * d1 + col2 * d2 + col3 * d3;

        uv1 = fract(uv1 * 1.5);
        uv1 -= 0.5;
        uv2 = fract(uv2 * 1.5);
        uv2 -= 0.5;
        uv3 = fract(uv3 * 1.5);
        uv3 -= 0.5;
    }

    outputColor = vec4(finalColor, 1.0);
}
