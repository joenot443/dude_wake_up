#version 150

uniform sampler2D tex;
uniform vec2 dimensions;
uniform float time;
uniform float speed;
uniform float shapeDistance;

// New uniforms
uniform float u_count;
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

    for (float i = 0.0; i < u_count; i++) { // count: 20.0 -> u_count
        float shapeDistance = abs(length(u * u) - i * shapeDistance * 0.1);

        vec3 color = (cos(i + vec3(u_colorOffsetR, u_colorOffsetG, u_colorOffsetB)) + 1.0); // 0.0: r, 1.0: g, 2.0: b -> uniforms
        float animation = smoothstep(0.35, 0.4, abs(abs(mod(time*speed, u_rate) - i * 0.1) - 1.0)); // 2.0: rate -> u_rate

        outputColor.rgb += 0.004 / shapeDistance * color * animation;

        float angle = (time*speed + i) * u_rotationOffset; // offset: 0.1 -> u_rotationOffset
        mat2 rotation = mat2(cos(angle), -sin(angle), sin(angle), cos(angle));
        u *= rotation;
    }
    outputColor.a = 1.0;
}
