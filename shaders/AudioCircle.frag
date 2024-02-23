#version 150

uniform float audio[256]; // Audio data
in vec2 coord;            // Screen coordinates
out vec4 outputColor;     // Final color output
uniform float time; // Uniform to represent time for background modulation

const float radius = 0.5; // Radius of the circle
const int bars = 256;     // Number of bars around the circle
const float PI = 3.14159;


// Function to create a smooth gradient background
vec3 pastelBackground(vec2 position) {
    // Modulate colors smoothly across the screen
    float noise = sin(position.x * 10.0 + time) * cos(position.y * 10.0 - time);
    vec3 color = vec3(0.9, 0.7, 0.8) + vec3(0.1, 0.2, 0.3) * noise;
    return color * 0.5 + 0.5; // Ensure the colors stay in a pleasant range
}

// Smooth interpolation of audio data
float getAudioHeight(float angle) {
    float normalizedAngle = (angle + PI) / (2.0 * PI);
    float exactIndex = normalizedAngle * bars;
    int index = int(exactIndex) % bars;
    int nextIndex = (index + 1) % bars;
    float interp = fract(exactIndex);
    return mix(audio[index], audio[nextIndex], interp);
}

void main() {
    // Normalize coordinates around the center
    vec2 normCoord = coord - vec2(0.5, 0.5);
    normCoord *= 2.0; // Scale to -1 to 1 range

    // Calculate angle and radius for polar coordinates
    float angle = atan(normCoord.y, normCoord.x);
    float dist = length(normCoord);

    // Get smoothed audio height
    float audioHeight = getAudioHeight(angle);

    // Define the curve's height based on smoothed audio data
    float curveHeight = 0.05 + audioHeight * 0.15;

    // Smooth curve around the circle
    float curve = smoothstep(radius, radius + curveHeight, dist) - smoothstep(radius + curveHeight, radius + curveHeight + 0.05, dist);

    // Background
    vec3 background = pastelBackground(normCoord);

    // Color variation based on audio data
    vec3 color = mix(vec3(0.5, 0.8, 1.0), vec3(1.0, 0.2, 0.3), audioHeight);

    // Apply the color if within the curve, otherwise show background
    if (curve > 0.0) {
        outputColor = vec4(color * curve, 1.0);
    } else {
        outputColor = vec4(background, 1.0); // Pastel background
    }
}
