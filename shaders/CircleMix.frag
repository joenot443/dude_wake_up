#version 150

uniform sampler2D tex;
uniform sampler2D tex2;
uniform vec2 dimensions;
uniform float time;
uniform float radius;
uniform float warp;
uniform float feather;
in vec2 coord;
out vec4 outputColor;

void main()
{
    // Get the base texture (background)
    vec4 baseColor = texture(tex, coord);
    
    // Get the overlay texture (circle content)
    vec4 overlayColor = texture(tex2, coord);
    
    // Convert texture coordinates to screen space (-1 to 1)
    vec2 screenPos = (coord - 0.5) * 2.0;
    
    // Apply aspect ratio correction based on dimensions
    float aspectRatio = dimensions.x / dimensions.y;
    screenPos.x *= aspectRatio;
    
    // Apply warp parameter to control circle distortion
    // warp = 1.0 gives perfect circle, other values create ellipses
    screenPos.x *= mix(1.0, 1.0 / aspectRatio, warp);
    
    // Calculate distance from center
    float dist = length(screenPos);
    
    // Create circular mask with feathered edges
    float circleRadius = radius * 2.0; // Scale radius to reasonable range
    
    // Feather controls the gradient width - larger values = softer transition
    float featherWidth = feather * 0.5; // Scale feather parameter
    float innerRadius = circleRadius - featherWidth;
    float outerRadius = circleRadius + featherWidth;
    
    // Create smooth gradient mask using smoothstep
    float mask;
    if (featherWidth > 0.001) {
        // Smooth gradient transition
        mask = 1.0 - smoothstep(innerRadius, outerRadius, dist);
    } else {
        // Sharp edge fallback
        mask = dist <= circleRadius ? 1.0 : 0.0;
    }
    
    // Ensure mask stays in valid range
    mask = clamp(mask, 0.0, 1.0);
    
    // Mix the two textures based on the feathered circular mask
    outputColor = mix(baseColor, overlayColor, mask);
}



