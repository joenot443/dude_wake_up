#version 150

uniform sampler2D tex;
uniform vec2 dimensions;
uniform float time;
uniform float shapeDistance;
in vec2 coord;
out vec4 outputColor;

// "The Core" by @kishimisu (2023) - https://www.shadertoy.com/view/cdy3Dd
void main() {
    vec2 r = dimensions.xy;
    vec2 u = (coord + coord - r) / r.y;
    outputColor = vec4(0.0); // Initialize the output color to zero

    for (float i = 0.0; i < 20.0; i++) {
        float shapeDistance = abs(length(u * u) - i * shapeDistance * 0.1);
        vec3 color = (cos(i + vec3(0.0, 1.0, 2.0)) + 1.0);
        float animation = smoothstep(0.35, 0.4, abs(abs(mod(time, 2.0) - i * 0.1) - 1.0));

        outputColor.rgb += 0.004 / shapeDistance * color * animation;

        float angle = (time + i) * 0.03;
        mat2 rotation = mat2(cos(angle), -sin(angle), sin(angle), cos(angle));
        u *= rotation;
    }

    outputColor.a = 1.0; // Set the alpha component to 1.0
}
