#version 150

// Uniforms (Inputs from the application)
uniform sampler2D tex;      // Texture sampler (unused in this shader logic)
uniform vec2 dimensions; // Dimensions of the render target (e.g., screen width and height)
uniform float time;       // Time varying value, likely for animation
uniform float distancePower; // Controls the exponent for distance calculation
uniform float radiusMultiplier; // Multiplier for the base radius
uniform float redMultiplier;    // Multiplier for the red color component
uniform float greenMultiplier;  // Multiplier for the green color component
uniform float blueMultiplier;   // Multiplier for the blue color component

// Inputs (Varying variables from the vertex shader)
in vec2 coord; // Input texture coordinates or fragment coordinates

// Outputs (Color written to the render target)
out vec4 outputColor; // Final fragment color

// Macros
#define n_tau 6.2831 // Define Tau (2 * PI)

/**
 * @brief Generates a pseudo-random float between 0.0 and 1.0 based on an input float.
 * @param seed The input float used as a seed.
 * @return A pseudo-random float in the range [0.0, 1.0).
 */
float hash11(float seed)
{
    // Simple hash function using fractional parts and multiplications
    seed = fract(seed * 0.1031);
    seed *= seed + 33.33;
    seed *= seed + seed;
    return fract(seed);
}

void main()
{
    // Calculate normalized coordinates, centered at (0,0), aspect ratio corrected
    vec2 uvCentered = (coord.xy - dimensions.xy * 0.5) / dimensions.y;

    vec3 totalColor = vec3(0.0); // Accumulator for the final color
    // Define a palette of colors
    vec3[] colorPalette = vec3[] (
        vec3(0.086 * redMultiplier, 0.208 * greenMultiplier, 0.816 * blueMultiplier),
        vec3(0.243 * redMultiplier, 0.639 * greenMultiplier, 0.435 * blueMultiplier),
        vec3(0.565 * redMultiplier, 0.282 * greenMultiplier, 0.796 * blueMultiplier),
        vec3(0.863 * redMultiplier, 0.878 * greenMultiplier, 0.102 * blueMultiplier),
        vec3(0.655 * redMultiplier, 0.216 * greenMultiplier, 0.184 * blueMultiplier)
    );
    float paletteSize = float(colorPalette.length()); // Get the number of colors in the palette

    float numIterations = 100.0; // Number of points to generate and check
    float minDistance = 1.0;     // Initialize minimum distance found to a high value
    vec3 minDistanceColor = vec3(0.0); // Color associated with the closest point
    float distanceProduct = 1.0; // Accumulator for the product of distances

    // Loop to generate points and calculate color contributions
    for (float i = 0.0; i < numIterations; i += 1.0) {
        float t = i / numIterations; // Normalized iteration progress [0.0, 1.0)

        // Calculate a radius that varies pseudo-randomly over time and iteration
        float radius = (sin(time + t * n_tau * 2324.2) * 0.5 + 0.5) * radiusMultiplier; // Use radiusMultiplier
        // Calculate the angle based on iteration progress
        float angle = n_tau * t;

        // Generate a pseudo-random value based on iteration progress
        float randomValue = hash11(t);
        // Select a color from the palette using the random value
        vec3 currentColor = colorPalette[int(randomValue * paletteSize)];

        // Calculate the position of the current point in a spiral pattern
        vec2 pointPosition = vec2(
            sin(angle) * radius,
            cos(angle) * radius
        );

        // Calculate the distance from the current fragment to the generated point
        float distance = length(pointPosition - uvCentered);

        // Modify the distance (non-linear mapping)
        distance = pow(distance, distancePower); // Use the uniform here

        // Add the current point's color contribution to the total, weighted by distance
        // The smoothstep creates a fade effect based on distance and the random value
        totalColor += currentColor * smoothstep(randomValue, 0.0, distance);

        // Keep track of the minimum distance found and the color associated with it
        if (distance < minDistance) {
            minDistance = distance;
            minDistanceColor = currentColor; // Store the color of the closest point
        }

        // Accumulate the product of all distances
        distanceProduct *= distance;
    }

    // Calculate a final factor based on the minimum distance
    float finalDistanceFactor = 1.0 - pow(minDistance, 1.0 / 3.0);
    // Apply smoothing to the factor
    finalDistanceFactor = smoothstep(0.3, 1.0, finalDistanceFactor);

    // Calculate the final output color
    // Combine the summed colors, modulated by the distance product and scaled
    // The exact visual effect depends heavily on these final calculations
    outputColor = vec4(totalColor * pow(distanceProduct, 1.0 / 22.0) * 4.0, 1.0);
}