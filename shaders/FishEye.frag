#version 150

uniform sampler2D tex;      // Input texture
uniform vec2 dimensions;   // Dimensions of the input texture (width, height)
uniform float time;        // Unused in this version, but kept for interface compatibility
uniform float amount;      // Effect amount (suggested range: 0.0 to 1.0, adjust as needed)

in vec2 coord;             // Input fragment coordinates (from vertex shader, usually pixel coordinates)
out vec4 outputColor;      // Output color for this fragment

const float PI = 3.141592653589793;

void main() {
  // 1. Normalize coordinates to [0.0, 1.0] range based on actual dimensions
  vec2 p = coord.xy / dimensions.xy;
  
  // 2. Define the center (always 0.5, 0.5 in normalized coordinates)
  vec2 center = vec2(0.5, 0.5);
  
  // 3. Calculate the aspect ratio
  float aspectRatio = dimensions.x / dimensions.y;
  
  // 4. Calculate the vector from the center to the current fragment
  vec2 d_raw = p - center;
  
  // 5. **Aspect Ratio Correction**: Scale the coordinate space so distances are circular.
  //    We scale the x-component of the difference vector by the aspect ratio.
  //    This effectively transforms the space into a conceptual square for distance/angle calculations.
  vec2 d_corrected = d_raw * vec2(aspectRatio, 1.0);
  
  // 6. Calculate the distance 'r' from the center in the aspect-corrected space.
  float r = length(d_corrected);
  
  // 7. Calculate the maximum possible distance in the aspect-corrected space (distance to corner)
  //    This is used to normalize the effect strength consistently across different aspect ratios.
  float max_r = length(vec2(0.5 * aspectRatio, 0.5));
  
  // 8. Calculate the fisheye effect strength ('power')
  //    Map 'amount' (e.g., 0.0 to 1.0) to a suitable range for the formulas.
  //    Here, amount=0.5 is no effect, <0.5 is anti-fisheye (barrel), >0.5 is fisheye (pincushion).
  //    The multiplier (e.g., 4.0) controls the intensity range. Adjust as needed.
  float power = (amount - 0.5) * 4.0;
  
  // 9. Calculate the distorted radius ('new_r') based on the original formulas,
  //    but using the aspect-corrected distance 'r' and normalizing factor 'max_r'.
  float new_r;
  
  if (abs(power) < 0.001) { // No effect (or very close to it)
    new_r = r;
  } else if (power > 0.0) { // Fisheye (Pincushion)
    // Ensure the argument inside atan is safe, avoid division by zero if max_r*power is a multiple of PI/2 (unlikely with floats but good practice)
    float tan_max_effect = tan(max_r * power * 0.5 * PI); // Using PI/2 scaling from original idea? Let's simplify to common model
    // Common fisheye formula: atan mapping
    new_r = atan(r * power * 2.0) / atan(max_r * power * 2.0) * max_r; // Multiply power for stronger effect range
  } else { // Anti-Fisheye (Barrel)
    // Common barrel distortion formula: tan mapping
    // Clamp values inside tan to avoid asymptotes if necessary, depending on power range
    // Ensure tan(max_r * power) is not zero
    float max_r_power = max_r * power; // power is negative here
    if (abs(tan(max_r_power)) < 0.0001) {
      new_r = r; // Avoid division by zero, fallback to no distortion
    } else {
      new_r = tan(r * power) / tan(max_r_power) * max_r;
    }
  }
  
  // 10. Calculate the final texture coordinate 'uv'
  vec2 uv;
  if (r == 0.0) { // Avoid division by zero when normalizing d_corrected if r is 0
    uv = center;
  } else {
    // Calculate the distorted vector in the aspect-corrected space
    vec2 distorted_d_corrected = normalize(d_corrected) * new_r;
    
    // Transform the distorted vector *back* from the corrected space to the original normalized texture space
    vec2 distorted_d_raw = distorted_d_corrected / vec2(aspectRatio, 1.0);
    
    // Add the distorted vector back to the center
    uv = center + distorted_d_raw;
  }
  
  // 11. Sample the texture using the calculated 'uv' coordinates
  //     Optional: Add clamping or boundary checks for uv if needed
  //     uv = clamp(uv, 0.0, 1.0); // Uncomment this to prevent sampling outside texture bounds
  
  vec3 col = texture(tex, uv).rgb;
  
  // 12. Set the output color
  outputColor = vec4(col, 1.0);
}
