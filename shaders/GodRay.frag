#version 150

uniform sampler2D tex;
uniform vec2 dimensions;
uniform float time;
uniform float colorMix;
uniform int colorTransparentPixels;
uniform vec4 color;
in vec2 coord;
out vec4 outputColor;

uniform float angle = -0.3;
uniform float position = -0.2;
uniform float spread = 0.5;
uniform float cutoff = 0.1;
uniform float falloff = 0.2;
uniform float edge_fade = 0.15;

uniform float speed = 1.0;
uniform float ray1_density = 8.0;
uniform float ray2_density = 30.0;
uniform float ray2_intensity = 0.3;

uniform float seed = 5.0;

float random(vec2 _uv) {
    return fract(sin(dot(_uv.xy, vec2(12.9898, 78.233))) * 43758.5453123);
}

float noise(vec2 uv) {
    vec2 i = floor(uv);
    vec2 f = fract(uv);

    float a = random(i);
    float b = random(i + vec2(1.0, 0.0));
    float c = random(i + vec2(0.0, 1.0));
    float d = random(i + vec2(1.0, 1.0));

    vec2 u = f * f * (3.0 - 2.0 * f);

    return mix(a, b, u.x) + (c - a) * u.y * (1.0 - u.x) + (d - b) * u.x * u.y;
}

mat2 rotate(float _angle){
    return mat2(vec2(cos(_angle), -sin(_angle)), vec2(sin(_angle), cos(_angle)));
}

void main() {
    vec2 transformed_uv = (rotate(angle) * (coord - position)) / ((coord.y + spread) - (coord.y * spread));

    vec2 ray1 = vec2(transformed_uv.x * ray1_density + sin(time * 0.1 * speed) * (ray1_density * 0.2) + seed, 1.0);
    vec2 ray2 = vec2(transformed_uv.x * ray2_density + sin(time * 0.2 * speed) * (ray1_density * 0.2) + seed, 1.0);

    float cut = step(cutoff, transformed_uv.x) * step(cutoff, 1.0 - transformed_uv.x);
    ray1 *= cut;
    ray2 *= cut;

    float rays = clamp(noise(ray1) + (noise(ray2) * ray2_intensity), 0., 1.);

    rays *= smoothstep(0.0, falloff, (1.0 - coord.y));
    rays *= smoothstep(cutoff, edge_fade + cutoff, transformed_uv.x);
    rays *= smoothstep(cutoff, edge_fade + cutoff, 1.0 - transformed_uv.x);

    vec4 t = texture(tex, coord);
    vec3 shine = vec3(rays) * color.rgb;

    outputColor = mix(t, vec4(shine, rays * color.a), colorMix);
}
