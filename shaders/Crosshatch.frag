#version 150

// Original Author: Florian Berger (flockaroo) - 2018
// License: Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License.
// Description: Crosshatch effect combining voxel-like and SDF aesthetics.

// Note: Consider enabling FAKE_VOXEL_AO in Buf A for potential performance gains.

// --- Constants ---
#define FLICKER_INTENSITY 0.0 // Controls flickering effect (currently disabled)
#define PI_TIMES_2 6.28318530718

// --- Global Variables (Modified in main, Used in getSampledColor) ---
vec2 globalRandomOffset;
float globalNoiseAmplitude;
float globalNoiseScaleFactor;

// --- Uniforms (Original Names Preserved + New Additions) ---
uniform sampler2D tex;             // Main input texture
uniform sampler2D rand;            // Noise texture
uniform float time;                // Time uniform for animation
uniform vec2 dimensions;          // Screen/render target dimensions
uniform float amount;              // Original amount uniform (affects hatch normalization)

// --- NEW UNIFORMS ---
// 1. Controls the base exponent for how outline noise scales per pass.
//    Default behavior uses 1.3. Values > 1 make later passes diverge faster.
//    Values closer to 1 make passes more similar.
uniform float outlineStyleExponentBase; // Example default: 1.3

// 2. Controls the quadratic factor for hatch angle progression.
//    Default behavior uses 0.08. 0 = linear angle steps. > 0 spreads later angles faster.
uniform float hatchAngleQuadraticFactor; // Example default: 0.08

// 3. Controls the "sharpness" of the hatch transition based on brightness.
//    Affects the smoothstep range. Default is 1.0 (0.5 to 1.5). Smaller values = sharper.
uniform float hatchSharpness; // Example default: 1.0, must be > 0

// 4. Sets the base color tint of the "paper".
//    Default behavior is white/grayscale (0.95). This allows color tinting.
uniform vec3 paperBaseColor; // Example default: vec3(0.95, 0.95, 0.92) for off-white

// 5. Controls the amplitude of the coordinate distortion based on noise in getSampledColor.
//    Default behavior uses 10.0. Higher = more "wiggle". 0 = no distortion.
uniform float noiseWiggleAmount; // Example default: 10.0

// --- Varyings ---
in vec2 coord; // Input texture coordinate from vertex shader

// --- Outputs ---
out vec4 outputColor; // Final fragment color

// --- Utility Functions ---

vec2 getSmoothedUV(vec2 uv, vec2 textureResolution)
{
  return uv + 0.6 * sin(uv * textureResolution * PI_TIMES_2) / PI_TIMES_2 / textureResolution;
}

vec4 sampleRandomTexture(vec2 position)
{
  vec2 randomTextureResolution = vec2(textureSize(rand, 0));
  vec2 uv = position / randomTextureResolution;
  uv = getSmoothedUV(uv, randomTextureResolution);
  return textureLod(rand, uv, 0.0);
}

vec4 getSampledColor(vec2 position)
{
  float baseScale = dimensions.x / 600.0;
  
  // Calculate noise-based offset using global parameters and the random texture
  // *** MODIFIED: Use noiseWiggleAmount uniform ***
  vec4 randomOffsetNoise = (sampleRandomTexture((position + globalRandomOffset) * 0.05 * globalNoiseScaleFactor / baseScale + time * 131.0 * FLICKER_INTENSITY) - 0.5) * noiseWiggleAmount * globalNoiseAmplitude;
  
  vec2 inputTextureResolution = vec2(textureSize(tex, 0));
  vec2 uv = (position + randomOffsetNoise.xy * baseScale) / dimensions.xy;
  // uv = getSmoothedUV(uv, inputTextureResolution);
  
  vec4 sampledColor = texture(tex, uv);
  
  // --- Original Background/Vignette Logic (Commented Out) ---
  // ... (kept as is) ...
  
  return sampledColor;
}

float getLuminance(vec2 position)
{
  return clamp(dot(getSampledColor(position).xyz, vec3(0.333)), 0.0, 1.0);
}

vec2 getGradient(vec2 position, float epsilon)
{
  vec2 offset = vec2(epsilon, 0.0);
  float luminanceX1 = getLuminance(position + offset.xy);
  float luminanceX0 = getLuminance(position - offset.xy);
  float luminanceY1 = getLuminance(position + offset.yx);
  float luminanceY0 = getLuminance(position - offset.yx);
  
  return vec2(luminanceX1 - luminanceX0, luminanceY1 - luminanceY0) / epsilon / 2.0;
}

// --- Main Shader Logic ---
void main()
{
  float baseScale = dimensions.x / 600.0;
  float scaledSqrtBaseScale = sqrt(baseScale);
  
  // --- Noise Preparation ---
  vec4 noiseSample1 = sampleRandomTexture(coord * 1.2 / scaledSqrtBaseScale)
  - sampleRandomTexture(coord * 1.2 / scaledSqrtBaseScale + vec2(1.0, -1.0) * 1.5);
  vec4 noiseSample2 = sampleRandomTexture(coord * 1.2 / scaledSqrtBaseScale);
  
  // --- Outline Effect ---
  float outlineIntensity = 0.0;
  globalRandomOffset = vec2(0.0);
  const int outlinePasses = 3;
  
  for (int i = 0; i < outlinePasses; i++)
  {
    float normalizedIndex = float(i) / float(outlinePasses - 1);
    float edgeThreshold = 0.03 + 0.25 * normalizedIndex;
    float edgeWidth = edgeThreshold * 2.0;
    float gradientEpsilon = 0.4 * baseScale;
    float gradientMagnitude = length(getGradient(coord, gradientEpsilon)) * baseScale;
    
    // *** MODIFIED: Use outlineStyleExponentBase uniform ***
    float passExponent = normalizedIndex * 5.0;
    float basePower = outlineStyleExponentBase; // Use the uniform
    
    // Pass 1: Closely matched edge line
    globalNoiseAmplitude = 0.15 * pow(basePower, passExponent);
    globalNoiseScaleFactor = 1.7 * pow(basePower, -passExponent);
    outlineIntensity += 0.6 * (0.5 + normalizedIndex) * smoothstep(edgeThreshold - edgeWidth / 2.0, edgeThreshold + edgeWidth / 2.0, gradientMagnitude);
    
    // Pass 2: Wildly varying edge line
    globalNoiseAmplitude = 0.3 * pow(basePower, passExponent);
    globalNoiseScaleFactor = 10.7 * pow(basePower, -passExponent);
    outlineIntensity += 0.4 * (0.2 + normalizedIndex) * smoothstep(edgeThreshold - edgeWidth / 2.0, edgeThreshold + edgeWidth / 2.0, gradientMagnitude);
  }
  
  outputColor.xyz = vec3(1.0) - 0.7 * outlineIntensity * (0.5 + 0.5 * noiseSample2.z) * 3.0 / float(outlinePasses);
  outputColor.xyz = clamp(outputColor.xyz, 0.0, 1.0);
  
  
  // --- Cross Hatch Effect ---
  globalNoiseAmplitude = 0.0;
  const int hatchLevels = 5;
#define N(v) (v.yx * vec2(-1, 1))
#define CS(ang) cos(ang - vec2(0, 1.5707963))
  
  float hatchAccumulator = 0.0;
  float maxHatchValue = 0.0;
  float hatchNormalizationSum = 0.0;
  
  for (int i = 0; i < hatchLevels; i++)
  {
    float flickerOffsetScale = clamp(FLICKER_INTENSITY, -1.0, 1.0);
    vec2 flickerOffset = 1.5 * baseScale * (sampleRandomTexture(coord * 0.02 + time * 1120.0).xy - 0.5) * flickerOffsetScale;
    float localBrightness = getLuminance(coord + flickerOffset) * 1.7;
    
    // Calculate hatch angle
    // *** MODIFIED: Use hatchAngleQuadraticFactor uniform ***
    float iSquared = float(i) * float(i);
    float hatchAngle = -0.5 - hatchAngleQuadraticFactor * iSquared; // Use uniform
    
    vec2 cosTheta = CS(hatchAngle);
    vec2 sinTheta = N(cosTheta);
    mat2 rotationMatrix = mat2(cosTheta, sinTheta);
    vec2 hatchUV = rotationMatrix * coord / scaledSqrtBaseScale * vec2(0.05, 1.0) * 1.3;
    
    vec4 hatchRandomSample = pow(sampleRandomTexture(hatchUV + 1003.123 * time * FLICKER_INTENSITY + vec2(sin(hatchUV.y), 0.0)), vec4(1.0));
    
    // Calculate hatch contribution
    // *** MODIFIED: Use hatchSharpness uniform ***
    float hatchLowerBound = 0.5;
    float hatchUpperBound = hatchLowerBound + max(0.01, hatchSharpness); // Ensure range > 0
    float currentHatch = 1.0 - smoothstep(hatchLowerBound, hatchUpperBound, (hatchRandomSample.x) + localBrightness) - 0.3 * abs(noiseSample1.z);
    
    hatchAccumulator += currentHatch;
    maxHatchValue = max(maxHatchValue, currentHatch);
    hatchNormalizationSum += 1.0 * amount; // Use original 'amount' uniform
    
    if (float(i) > (1.0 - localBrightness) * float(hatchLevels) && i >= 2)
    {
      break;
    }
  }
  
  float finalHatchFactor = 0.0;
  if (hatchNormalizationSum > 0.0) {
    finalHatchFactor = clamp(mix(hatchAccumulator / hatchNormalizationSum, maxHatchValue, 0.5), 0.0, 1.0);
  }
  outputColor.xyz *= (1.0 - finalHatchFactor);
  
  
  // --- Final Adjustments ---
  
  // Apply a contrast/brightness adjustment
  outputColor.xyz = 1.0 - ((1.0 - outputColor.xyz) * 0.7);
  
  // Apply paper texture using noise
  // *** MODIFIED: Use paperBaseColor uniform ***
  vec3 paperNoiseEffect = 0.06 * noiseSample1.xxx + 0.06 * noiseSample1.xyz;
  outputColor.xyz *= paperBaseColor + paperNoiseEffect; // Apply noise relative to base color
  outputColor.w = 1.0;
  
  // Apply vignette effect
  vec2 normalizedCenterCoords = (coord - 0.5 * dimensions.xy) / dimensions.x;
  float vignetteFactor = 1.0 - 0.3 * dot(normalizedCenterCoords, normalizedCenterCoords);
  vignetteFactor *= 1.0 - 0.7 * exp(-sin(coord.x / dimensions.x * 3.1415926) * 40.0);
  vignetteFactor *= 1.0 - 0.7 * exp(-sin(coord.y / dimensions.y * 3.1415926) * 20.0);
  outputColor.xyz *= vignetteFactor;
  
  outputColor = clamp(outputColor, 0.0, 1.0);
}
