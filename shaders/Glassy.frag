#version 150

uniform sampler2D tex;
uniform vec2 dimensions;
uniform float time;
uniform float specularPower;
uniform float reflectivity;
uniform float shapeSize;
uniform float distortion;
uniform float repetitionScale;
uniform float shapeBlend;
in vec2 coord;
out vec4 outputColor;

// Mirror-like "Happy Accident" Shader (CC0)
// A shiny reflective variation of a raymarched fractal accident

float map(vec3 p) {
    // Domain repetition with configurable scale
    p *= repetitionScale;
    p = abs(fract(p) - 0.5);
    // Cylinder + planes SDF
    float cylPlanes = abs(min(length(p.xy) - shapeSize, min(p.x, p.y) + 1e-3)) + 1e-3;
    // Sphere SDF
    float sphere = abs(length(p.xyz) - shapeSize) + 1e-3;
    // Blend between shapes
    return mix(cylPlanes, sphere, shapeBlend);
}

vec3 estimateNormal(vec3 p) {
    float eps = 0.001;
    return normalize(vec3(
        map(p + vec3(eps, 0.0, 0.0)) - map(p - vec3(eps, 0.0, 0.0)),
        map(p + vec3(0.0, eps, 0.0)) - map(p - vec3(0.0, eps, 0.0)),
        map(p + vec3(0.0, 0.0, eps)) - map(p - vec3(0.0, 0.0, eps))
    ));
}

void main() {
    vec2 r = dimensions.xy;
    vec2 uv = (coord - 0.5 * r) / r.y;

    float t = time;
    float z = fract(dot(coord, sin(coord))) - 0.5;
    vec4 col = vec4(0.0);
    vec4 p;

    for (float i = 0.0; i < 77.0; i++) {
        // Ray direction
        p = vec4(z * normalize(vec3(coord - 0.7 * r, r.y)), 0.1 * t);
        p.z += t;

        vec4 q = p;

        // Configurable distortion rotation matrices
        p.xy *= mat2(cos(distortion + q.z + vec4(0,11,33,0)));
        p.xy *= mat2(cos(q + vec4(0,11,33,0)));

        // Distance estimation
        float d = map(p.xyz);

        // Estimate lighting
        vec3 pos = p.xyz;
        vec3 lightDir = normalize(vec3(0.3, 0.5, 1.0));
        vec3 viewDir = normalize(vec3(uv, 1.0));
        vec3 n = estimateNormal(pos);
        vec3 reflectDir = reflect(viewDir, n);

        // Fake environment reflection
        vec3 envColor = mix(vec3(0.8, 0.4, 0.8), vec3(1.0), 0.5 + 0.5 * reflectDir.y);

        // Specular highlight
        float spec = pow(max(dot(reflectDir, lightDir), 0.0), specularPower);

        // Funky palette color
        vec4 baseColor = (1.0 + sin(0.5 * q.z + length(p.xyz - q.xyz) + vec4(0,4,3,6)))
                       / (0.5 + 2.0 * dot(q.xy, q.xy));

        // Combine base color + environment reflection + specular highlight
        vec3 finalColor = baseColor.rgb * (1.0 - reflectivity) + envColor * reflectivity + vec3(spec) * 1.2;

        // Brightness weighted accumulation
        col.rgb += finalColor / d;

        z += 0.6 * d;
    }

    // Compress brightness range
    outputColor = vec4(tanh(col.rgb / 2e4), 1.0);
}
