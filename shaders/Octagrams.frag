#version 150

uniform sampler2D tex;
uniform vec2 dimensions;
uniform float time;
in vec2 coord;
out vec4 outputColor;

precision highp float;

// Global time variable used in box animations
float globalTime = 0.0;

// Rotation matrix helper function
mat2 createRotationMatrix(float angle) {
  float cosAngle = cos(angle);
  float sinAngle = sin(angle);
  return mat2(cosAngle, sinAngle, -sinAngle, cosAngle);
}

// Signed distance function for a box
float signedDistanceBox(vec3 position, vec3 boxSize) {
  vec3 q = abs(position) - boxSize;
  return length(max(q, 0.0)) + min(max(q.x, max(q.y, q.z)), 0.0);
}

// Creates a single box with scaling and rotation
float createBox(vec3 position, float scale) {
  position *= scale;
  float baseBox = signedDistanceBox(position, vec3(0.4, 0.4, 0.1)) / 1.5;
  position.xy *= 5.0;
  position.y -= 3.5;
  position.xy *= createRotationMatrix(0.75);
  return -baseBox;
}

// Creates a set of boxes with time-based animations
float createBoxSet(vec3 position, float time) {
  vec3 originalPos = position;
  
  // Create four boxes with different positions and animations
  position = originalPos;
  position.y += sin(globalTime * 0.4) * 2.5;
  position.xy *= createRotationMatrix(0.8);
  float box1 = createBox(position, 2.0 - abs(sin(globalTime * 0.4)) * 1.5);
  
  position = originalPos;
  position.y -= sin(globalTime * 0.4) * 2.5;
  position.xy *= createRotationMatrix(0.8);
  float box2 = createBox(position, 2.0 - abs(sin(globalTime * 0.4)) * 1.5);
  
  position = originalPos;
  position.x += sin(globalTime * 0.4) * 2.5;
  position.xy *= createRotationMatrix(0.8);
  float box3 = createBox(position, 2.0 - abs(sin(globalTime * 0.4)) * 1.5);
  
  position = originalPos;
  position.x -= sin(globalTime * 0.4) * 2.5;
  position.xy *= createRotationMatrix(0.8);
  float box4 = createBox(position, 2.0 - abs(sin(globalTime * 0.4)) * 1.5);
  
  // Add two additional boxes
  position = originalPos;
  position.xy *= createRotationMatrix(0.8);
  float box5 = createBox(position, 0.5) * 6.0;
  
  position = originalPos;
  float box6 = createBox(position, 0.5) * 6.0;
  
  return max(max(max(max(max(box1, box2), box3), box4), box5), box6);
}

// Main distance field function
float map(vec3 position, float time) {
  return createBoxSet(position, time);
}

void main() {
  // Normalize coordinates to [-1, 1] range
  vec2 normalizedCoord = (coord.xy * 2.0 - dimensions.xy) / min(dimensions.x, dimensions.y);
  
  // Set up ray marching parameters
  vec3 rayOrigin = vec3(0.0, -0.2, time * 4.0);
  vec3 rayDirection = normalize(vec3(normalizedCoord, 1.5));
  
  // Apply time-based rotations to ray
  rayDirection.xy = rayDirection.xy * createRotationMatrix(sin(time * 0.03) * 5.0);
  rayDirection.yz = rayDirection.yz * createRotationMatrix(sin(time * 0.05) * 0.2);
  
  float distance = 0.1;
  vec3 color = vec3(0.0);
  float accumulation = 0.0;
  
  // Ray marching loop
  for (int i = 0; i < 33; i++) {
    vec3 position = rayOrigin + rayDirection * distance;
    position = mod(position - 2.0, 4.0) - 2.0;
    globalTime = time - float(i) * 0.01;
    
    float d = map(position, time);
    d = max(abs(d), 0.01);
    accumulation += exp(-d * 23.0);
    
    distance += d * 0.55;
  }
  
  // Final color calculation
  color = vec3(accumulation * 0.02);
  color += vec3(0.0, 0.2 * abs(sin(time)), 0.5 + sin(time) * 0.2);
  
  // Output with time-based transparency
  outputColor = vec4(color, 1.0 - distance * (0.02 + 0.02 * sin(time)));
}
