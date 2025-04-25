#version 150

uniform float audio[256];
uniform sampler2D tex;
uniform vec2 dimensions;
uniform float ballDistance;
uniform float amount;
uniform float time;
in vec2 coord;
out vec4 outputColor;

float fft(float x) {
  // Fixed length of the audio array
  const int audioLength = 256;
  
  // Scale x to fit the range of the array (0 to audioLength - 1)
  float scaledX = x * float(audioLength - 1);
  
  // Get the integer part of scaledX
  int i = int(scaledX);
  
  // Ensure i is within bounds
  if (i < 0 || i >= audioLength - 1) {
    return 0.0; // Return a default value for out-of-bounds
  }
  
  // Calculate the fractional part
  float frac = scaledX - float(i);
  
  // Access audio data directly from the uniform array
  float audioValue1 = audio[i];
  float audioValue2 = audio[i + 1];
  
  // Perform linear interpolation between audio[i] and audio[i + 1]
  return mix(audioValue1, audioValue2, frac);
}

void main()
{
  vec3 r = vec3(1, 0, 0);
  vec3 y = vec3(1, 1, 0);
  vec3 g = vec3(0, 1, 0);
  vec3 b = vec3(0, 0, 1);
  vec3 ca[19];
  
  ca[0] = vec3(0.0);  // Initialize index 0 to avoid issues
  ca[1] = b * 2.0 / 6.0 + r * 4.0 / 6.0;
  ca[2] = b * 1.0 / 6.0 + r * 5.0 / 6.0;
  ca[3] = r;
  ca[4] = r * 5.0 / 6.0 + y / 6.0;
  ca[5] = r * 4.0 / 6.0 + y * 2.0 / 6.0;
  ca[6] = r * 3.0 / 6.0 + y * 3.0 / 6.0;
  ca[7] = r * 2.0 / 6.0 + y * 4.0 / 6.0;
  ca[8] = r * 1.0 / 6.0 + y * 5.0 / 6.0;
  ca[9] = y;
  ca[10] = y * 2.0 / 3.0 + g * 1.0 / 3.0;
  ca[11] = y * 1.0 / 3.0 + g * 2.0 / 3.0;
  ca[12] = g;
  ca[13] = g * 2.0 / 3.0 + b * 1.0 / 3.0;
  ca[14] = g * 1.0 / 3.0 + b * 2.0 / 3.0;
  ca[15] = b;
  ca[16] = b * 5.0 / 6.0 + r * 1.0 / 6.0;
  ca[17] = b * 4.0 / 6.0 + r * 2.0 / 6.0;
  ca[18] = b * 3.0 / 6.0 + r * 3.0 / 6.0;
  
  vec2 uv = (coord - 0.5 * dimensions.xy) / dimensions.y;
  outputColor = vec4(0.0);  // Initialize the output color
  
  float sinTime = sin(mod(time, 60.0) * 0.2);  // Initialize sinTime properly
  
  int c = 0;
  for (float x = 0.0; x < 3.14159265359 * 2.0; x += 3.14159265359 / 18.0) {
    c = (c + 1) % 18;
    vec4 color = vec4(ca[c], 1.0);  // Use vec3 ca and set alpha to 1.0
    
    vec2 sinVec = vec2(sin(x), cos(x));
    float pos1 = 1.5;
    float pos2 = 1.5 + ballDistance;
    float len1 = length(uv - 0.1 * sinTime * (x + pos1) * sinVec) + 0.0001;
    float len2 = length(uv - 0.1 * sinTime * (x + pos2) * sinVec) + 0.0001;
    
    float fftVal = min(0.5, fft(x / (3.14159265359 * 2.0)));  // Placeholder for fft value
    
    outputColor += color * vec4(amount * fftVal * x / 2.0 * 0.008 / len1);
    outputColor += color * vec4(amount * fftVal * x / 2.0 * 0.008 / len2);
  }
}
