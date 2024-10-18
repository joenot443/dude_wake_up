#version 150

// Uniform variables passed from the application
uniform sampler2D tex;          // A 2D texture sampler (unused in this shader)
uniform vec2 dimensions;        // The dimensions of the screen or texture
uniform float time;             // Time variable used for animation
uniform float brightness;             // Time variable used for animation
uniform float zoom;

// Inputs and outputs for the shader
in vec2 coord;                  // The input texture coordinates
out vec4 outputColor;           // The final output color of the fragment

// A function that calculates the falloff of a metaball's influence based on distance
float falloff(float x) {
    // Clamping the input between 0 and 1
    x = clamp(0., 1., x);
    // Applying a custom falloff curve by using exponential functions
    return pow(1. - pow(x, 8.), 80.3);
}

// Function that computes the origin (position) of a metaball based on a grid ID and time
vec2 getOrigin(vec2 id, float t) {
    // Returns an origin position based on sine and cosine functions with time to animate the metaballs
  return vec2(.3) + vec2(
      sin(t + id.y * 23.3 * id.y * 80.9 + id.y * 5.),
      cos(t + id.x * 79.1 + id.x)
  );
}

// Function that computes a color for a metaball based on its grid ID and time
vec3 getColor(vec2 id, float t) {
    // Creates a gradient factor based on the sine of the ID and time
    float gradientFactor = 0.5 + 0.5 * sin(id.x + id.y + t);
    // Defines two colors: purple and blue
    vec3 purple = vec3(0.6, 0.0, 1.0);
    vec3 blue = vec3(0.0, 0.0, 1.0);
    // Returns a color that transitions between purple and blue based on the gradient factor
    return mix(purple, blue, gradientFactor);
}

// Function that generates the metaball effect by iterating through multiple metaballs
vec3 MetaballLayer(vec2 uv, float t, float s) {
    vec3 m = vec3(0);           // Accumulator for metaball influence
    vec3 col = vec3(0);         // Color accumulator for the final output
    vec2 gv = fract(uv * s);    // Fractional part of the UV coordinates, for local grid

    // Looping over a grid of potential metaball positions (from -5 to 2 in both x and y)
    for (float x = -5.; x <= 2.; x++) {
        for (float y = -5.; y <= 2.; y++) {
            vec2 offset = vec2(x, y);  // Current grid offset
            
            // Calculate the grid ID for this metaball based on UV coordinates and scaling
            vec2 id = ((uv * s) - gv) + offset;
            // Get the origin (position) of the metaball for this grid cell
            vec2 o = getOrigin(id, t);
            // Calculate the distance from the metaball's origin to the current pixel
            vec2 d = (gv - offset) - o;
            float r = length(d);       // Compute the distance
            // Apply the falloff function to the distance to determine metaball influence
            r = falloff(r * 1.3 + 0.2 * cos(id.x * id.y)) * brightness;
            // Add the metaball's contribution to the final color based on its influence
            m += getColor(id, t) * r;
        }
    }
    
    col = m;  // Set the color based on the accumulated metaball contributions
    // If the metaball influence exceeds a threshold, normalize the color to limit its intensity
    if (length(m) > 0.6) {
       col = normalize(m);
    }
    return col;  // Return the final color for this layer
}

void main() {
    // Normalize the UV coordinates to make them relative to the screen size
    vec2 uv = ((coord - 0.2 * dimensions.xy) / dimensions.y) * zoom;
    
    vec3 col = vec3(0.5);  // Initialize the output color with a default gray
    
    float t = 0.5 * time;  // Scale the time for slower animation
    float size = 2.0 + 3.0 * cos(t * 0.2);  // Calculate metaball size variation over time
    
    // Create a 2D rotation matrix for rotating the metaballs over time
    float c = cos(t * 0.5);
    float s = sin(t * 0.5);
    mat2 rot = mat2(c, s, -s, c);  // Standard 2D rotation matrix
    
    // Apply the metaball layer, rotating the UV coordinates and scaling the result
    col = 1.3 * MetaballLayer(uv * rot, t, size);
    
    // Output the final color as a vec4, with the alpha channel set to 1 (fully opaque)
    outputColor = vec4(col, 1.0);
}
