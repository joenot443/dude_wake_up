#version 150

// Uniforms: Input data from the CPU/application
uniform vec2 dimensions; // Renamed from 'dimensions' for clarity (e.g., screen width, height)
uniform float time;      // Input time for animation

// Varying input: Data passed from the vertex shader
in vec2 coord; // Renamed from 'coord' (pixel coordinates)

// Output: The final color of the fragment (pixel)
out vec4 outputColor; // Renamed from 'outputColor'

// Constants for clarity (approximating golfed values where needed)
const float LOOP_ITERATIONS = 18.0; // Golfed used ++i < 19. (runs for i=1 to 18)
const float INITIAL_SCALE_ACC = 0.5;
const float SCALE_ACC_INCREMENT = 0.03;
const vec4 COLOR_COS_OFFSET = vec4(0.0, 1.0, 3.0, 0.0); // From original code comment, golfed version used accumulating color here which seemed unintended.
const float UV_INITIAL_SCALE = 0.2;
const float PI = 3.14159265359;

// Small value to prevent division by zero
const float EPSILON = 1e-6;

void main()
{
  // 1. Normalize coordinates: Map pixel coords to a centered space, scaled by height.
  // vec2 uv = UV_INITIAL_SCALE * (2.0 * coord - dimensions) / dimensions.y;
  // Simpler equivalent:
  vec2 uv = (coord - 0.5 * dimensions) / dimensions.y * (2.0 * UV_INITIAL_SCALE);
  
  // 2. Initialize accumulator variables
  vec4 accumulatedColor = vec4(1.0, 2.0, 3.0, 0.0); // Initial color from golfed code
  float scaleAccumulator = INITIAL_SCALE_ACC;
  float currentTime = time; // Copy time to modify it inside the loop
  
  // This variable 'v' is confusingly named. It starts as dimensions,
  // but is immediately overwritten in the first loop iteration's calculations
  // (specifically, the 'dot(v,v)' uses the initial value, then 'v' is recalculated).
  // Let's rename the calculated value inside the loop.
  vec2 v_calc = dimensions; // Used for dot(v_calc, v_calc) in the first iteration color calc
  
  // 3. Iterative process (Raymarching-like loop)
  // The original loop used ++i < 19., meaning i goes from 1 to 18.
  for (float i = 1.0; i < LOOP_ITERATIONS + 1.0; i += 1.0) // Loop 18 times (i = 1, 2, ..., 18)
  {
    // --- Color Accumulation ---
    // Using the accumulating color in the cos as per the literal golfed code:
    vec4 colorCosTerm = 1.0 + cos(accumulatedColor + currentTime);
    
    // Denominator calculation: length((1.+i*dot(v,v)) * sin(...))
    float dot_v_v = dot(v_calc, v_calc); // Uses value from *previous* iteration (or initial dimensions for i=1)
    float lengthScale = 1.0 + i * dot_v_v;
    
    // The term u/(.5-dot(u,u)) is related to stereographic projection / inversion
    float uvDistSq = dot(uv, uv);
    float inversionDenom = 0.5 - uvDistSq;
    // Avoid division by zero or near-zero
    inversionDenom = sign(inversionDenom) * max(abs(inversionDenom), EPSILON);
    vec2 sinArgBase = 1.5 * uv / inversionDenom - 9.0 * uv.yx + currentTime; // Golfed used 1.5 directly
    
    vec2 sinValue = sin(sinArgBase);
    float denominator = length(lengthScale * sinValue);
    
    // Avoid division by zero
    denominator = max(denominator, EPSILON);
    
    accumulatedColor += colorCosTerm / denominator;
    
    
    // --- Update Domain Variables (v_calc and uv) ---
    
    // Update 'v_calc' (was 'v' in golfed code's loop update)
    // Note: currentTime is incremented *before* being used here in the original (++t)
    currentTime += 1.0;
    scaleAccumulator += SCALE_ACC_INCREMENT;
    v_calc = cos(currentTime - 7.0 * uv * pow(scaleAccumulator, i)) - 5.0 * uv;
    
    // Update 'uv' (domain coordinate)
    // Apply standard 2D rotation
    float rotationAngle = i + 0.02 * currentTime;
    mat2 rotationMatrix = mat2(cos(rotationAngle), sin(rotationAngle), -sin(rotationAngle), cos(rotationAngle));
    uv = rotationMatrix * uv; // Apply rotation first
    
    // Add displacements to uv (Corrected Section)
    vec2 uv_yx = uv.yx;
    
    // Displacement 1 (tanh based)
    // Corrected: Added vec2(currentTime), changed tanhTerm to vec2, tanh now component-wise
    vec2 cosArg1 = 100.0 * uv_yx + vec2(currentTime);
    vec2 tanhTermVec = 40.0 * dot(uv, uv) * cos(cosArg1); // float * float * vec2 -> vec2
    vec2 displacement1 = tanh(tanhTermVec) / 200.0;       // tanh(vec2)/float -> vec2
    
    // Displacement 2 (scaled uv based)
    vec2 displacement2 = 0.2 * scaleAccumulator * uv; // float * float * vec2 -> vec2
    
    // Displacement 3 (color magnitude and time based)
    // Corrected: Changed displacement3 to vec2, broadcast float result
    float colorMagnitudeSq = dot(accumulatedColor, accumulatedColor);
    float displacement3Denom = exp(colorMagnitudeSq / 100.0);
    float cosArg3 = 4.0 / max(displacement3Denom, EPSILON) + currentTime; // Added epsilon
    float cosVal3 = cos(cosArg3);                // cos(float) -> float
    vec2 displacement3 = vec2(cosVal3) / 300.0;  // Broadcast float to vec2
    
    // Final uv update (vec2 + vec2 + vec2)
    uv += displacement1 + displacement2 + displacement3;
    
  } // End of for loop
  
  // 4. Final Color Transformation (Tone mapping / Post-processing)
  // This section uses the formula from the *golfed* code.
  
  float finalUVDistSq = dot(uv, uv);
  
  // Apply component-wise, adding epsilon to avoid division by zero
  vec4 clampedColor = min(accumulatedColor, vec4(13.0)); // Clamp max values
  vec4 divTerm = accumulatedColor + EPSILON; // Epsilon for 164./outputColor
  vec4 postProcessDenom = clampedColor + 164.0 / divTerm;
  // Avoid division by zero or small numbers for the final division
  postProcessDenom = mix(vec4(EPSILON), postProcessDenom, step(vec4(EPSILON), abs(postProcessDenom)));
  
  vec4 finalColorRGB = 25.6 / postProcessDenom;
  
  // Subtract vignetting term
  finalColorRGB -= vec4(finalUVDistSq / 250.0);
  
  outputColor = finalColorRGB;
  
  // Optional: Clamp final color to [0, 1] range if needed
  // fragColor = clamp(fragColor, 0.0, 1.0);
}
