#version 150

uniform sampler2D tex;
uniform vec2 dimensions;
uniform float time;
uniform float speed;
uniform float patternDensity;
uniform float twistAmount;
uniform float colorShift;
uniform float patternScale;

in vec2 coord;
out vec4 outputColor;

// Helper macro for sine wave normalization
#define normalizeSine(a) (sin(a) * 0.5 + 0.5)
#define timeOffset (time * 0.5 * speed)

// Box signed distance field function
float boxSDF(vec3 point, vec3 size) {
  vec3 q = abs(point) - size;
  return length(max(q, 0.0)) + min(max(q.x, max(q.y, q.z)), 0.0);
}

void main() {
    float iteration = 3.0;
    float radius = 3.0;
    float displacement = 3.0;
    
    vec2 normalizedCoord = dimensions.xy;
    
    // Ray marching loop
    for (iteration = 0.0; iteration < 150.0 && displacement > 0.002 * radius && radius < 50.0; iteration++) {
        // Calculate ray position with pattern scale
        vec3 rayPos = normalize(vec3((coord - normalizedCoord * 0.5) / normalizedCoord.y * (1.4 * patternScale), 1.0)) * radius;
        rayPos.z += timeOffset;
        
        // Calculate rotation angle based on time and twist amount
        float rotationAngle = mix(normalizeSine(timeOffset), 
                                -normalizeSine(timeOffset), 
                                normalizeSine((timeOffset - 3.14) / 2.0)) * radius * twistAmount;
        
        // Apply rotation matrix
        rayPos.xy *= mat2(cos(rotationAngle), sin(rotationAngle), 
                         -sin(rotationAngle), cos(rotationAngle));
        
        // Calculate displacement using box SDF with pattern density
        float boxSize = mix(0.2, 0.45, normalizeSine(timeOffset)) * patternDensity;
        displacement = max(boxSDF(fract(rayPos + 0.5) - 0.5, vec3(boxSize)), 
                         -boxSDF(rayPos, vec2(1.1, 1e9).xxy)) * 0.85;
        radius += displacement;
    }
    
    // Calculate final color based on radius, time, and color shift
    vec3 baseFreq = vec3(mix(2.05, 1.85, normalizeSine(timeOffset)), 2.1, 2.15);
    vec3 shiftedFreq = baseFreq + colorShift;
    vec3 color = cos(shiftedFreq * radius - timeOffset) * exp(-radius * 0.06);
    outputColor = vec4(color, 1.0);
}
