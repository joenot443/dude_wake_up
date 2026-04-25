#version 150

uniform sampler2D tex;
uniform vec2 dimensions;
uniform float time;
uniform float speed;
uniform float shapeDistance;

// New uniforms
uniform int u_count;
uniform float u_colorOffsetR;
uniform float u_colorOffsetG;
uniform float u_colorOffsetB;
uniform float u_rate;
uniform float u_rotationOffset;

in vec2 coord;
out vec4 outputColor;

// "The Core" by @kishimisu (2023) - https://www.shadertoy.com/view/cdy3Dd
void main() {
    vec2 r = dimensions.xy;
    vec2 u = (coord + coord - r) / r.y;
    outputColor = vec4(0.0); // Initialize the output color to zero

    for (int i = 0; i < u_count; i++) {
        float fi = float(i);
        float shapeDistance = abs(length(u * u) - fi * shapeDistance * 0.1);

        vec3 color = (cos(fi + vec3(u_colorOffsetR, u_colorOffsetG, u_colorOffsetB)) + 1.0);
        float animation = smoothstep(0.35, 0.4, abs(abs(mod(time*speed, u_rate) - fi * 0.1) - 1.0));

        outputColor.rgb += 0.004 / shapeDistance * color * animation;

        float angle = (time*speed + fi) * u_rotationOffset;
        mat2 rotation = mat2(cos(angle), -sin(angle), sin(angle), cos(angle));
        u *= rotation;
    }
    outputColor.a = 1.0;
}
