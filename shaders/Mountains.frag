#version 150

uniform sampler2D tex;
uniform vec2 dimensions;
uniform float time;
uniform vec4 color;
uniform vec3 mountainColor;
uniform vec3 valleyColor;
uniform float heightScale;
uniform float yTranslate;
uniform float zDistance;
uniform float rotateX;
uniform float rotateY;
uniform float rotateZ;
uniform float rotateW;
uniform float fade;
in vec2 coord;
out vec4 outputColor;

/////////////////////////////////////////////////////////////////////////////
// XBE
// Retro style terrain rendering
//

// Constants
const float PI = 3.141592654;
mat2 ROTATION_MATRIX = mat2(rotateX, rotateY, rotateZ, rotateW * 0.2);

// Noise functions from IQ
vec2 hash(vec2 p) {
  p = vec2(dot(p, vec2(127.1, 311.7)), dot(p, vec2(269.5, 183.3)));
  return -1.0 + 2.0 * fract(sin(p) * 43758.5453123);
}

float noise(in vec2 p) {
  const float K1 = 0.366025404;
  const float K2 = 0.211324865;
  
  vec2 i = floor(p + (p.x + p.y) * K1);
  vec2 a = p - i + (i.x + i.y) * K2;
  vec2 o = (a.x > a.y) ? vec2(1.0, 0.0) : vec2(0.0, 1.0);
  vec2 b = a - o + K2;
  vec2 c = a - 1.0 + 2.0 * K2;
  
  vec3 h = max(0.5 - vec3(dot(a, a), dot(b, b), dot(c, c)), 0.0);
  vec3 n = h * h * h * h * vec3(
                                dot(a, hash(i + 0.0)),
                                dot(b, hash(i + o)),
                                dot(c, hash(i + 1.0))
                                );
  
  return dot(n, vec3(70.0));
}

// Fractal Brownian Motion functions
float fbm4(in vec2 p) {
  float f = 0.0;
  f += 0.5000 * noise(p);
  p = ROTATION_MATRIX * p * 2.02;
  f += 0.2500 * noise(p);
  p = ROTATION_MATRIX * p * 2.03;
  f += 0.1250 * noise(p);
  p = ROTATION_MATRIX * p * 2.01;
  f += 0.0625 * noise(p);
  return f;
}

float fbm6(in vec2 p) {
  float f = 0.0;
  f += 0.5000 * noise(p);
  p = ROTATION_MATRIX * p * 2.02;
  f += 0.2500 * noise(p);
  p = ROTATION_MATRIX * p * 2.03;
  f += 0.1250 * noise(p);
  p = ROTATION_MATRIX * p * 2.01;
  f += 0.0625 * noise(p);
  p = ROTATION_MATRIX * p * 2.04;
  f += 0.031250 * noise(p);
  p = ROTATION_MATRIX * p * 2.01;
  f += 0.015625 * noise(p);
  return f;
}

// Camera and projection matrix helpers
mat4 createPerspectiveMatrix(in float fov, in float aspect, in float near, in float far) {
  mat4 m = mat4(0.0);
  float angle = (fov / 180.0) * PI;
  float f = 1.0 / tan(angle * 0.5);
  m[0][0] = f / aspect;
  m[1][1] = f;
  m[2][2] = (far + near) / (near - far);
  m[2][3] = -1.0;
  m[3][2] = (2.0 * far * near) / (near - far);
  return m;
}

mat4 createViewMatrix(vec3 eye, float pitch) {
  float cosPitch = cos(pitch);
  float sinPitch = sin(pitch);
  vec3 xaxis = vec3(1.0, 0.0, 0.0);
  vec3 yaxis = vec3(0.0, cosPitch, sinPitch);
  vec3 zaxis = vec3(0.0, -sinPitch, cosPitch);
  
  return mat4(
              vec4(xaxis.x, yaxis.x, zaxis.x, 0.0),
              vec4(xaxis.y, yaxis.y, zaxis.y, 0.0),
              vec4(xaxis.z, yaxis.z, zaxis.z, 0.0),
              vec4(-dot(xaxis, eye), -dot(yaxis, eye), -dot(zaxis, eye), 1.0)
              );
}

void main() {
  // Setup UV and normalized coordinates
  vec2 uv = vec2(coord.x, 1.0 - coord.y);
  vec2 normalizedPos = 2.0 * uv - 1.0;
  normalizedPos.x *= dimensions.x / dimensions.y;
  
  // Camera setup
  vec3 cameraPos = vec3(0.0, 0.1 + 0.25 * cos(0.5 * time), -1.0);
  mat4 projectionMatrix = createPerspectiveMatrix(50.0, dimensions.x / dimensions.y, 0.1, 10.0);
  mat4 viewMatrix = createViewMatrix(cameraPos, -5.0 * PI / 180.0);
  mat4 viewProjectionMatrix = viewMatrix * projectionMatrix;
  
  // Ray marching variables
  vec3 finalColor = vec3(0.0);
  vec3 accumulatedColor = vec3(0.0);
  float depth;
  vec4 position = vec4(0.0);
  float lastHeight = -dimensions.y;
  float timeOffset = 0.1 * time;
  float height;
  float zPos = 0.1;
  float zIncrement = zDistance * 0.05;
  
  // Ray marching loop
  for (int i = 0; i < 24; ++i) {
    position = vec4(
                    normalizedPos.x,
                    heightScale * fbm4(0.5 * vec2(cameraPos.x + normalizedPos.x, zPos + timeOffset)),
                    cameraPos.z + zPos,
                    yTranslate * 40.0
                    );
    
    height = (viewProjectionMatrix * position).y - normalizedPos.y;
    if (height > lastHeight) {
      depth = abs(height);
      float a = smoothstep(1.0, 0.0, depth * 186.0);
      finalColor = depth < 0.005 ? mix(mountainColor, valleyColor, uv.y) : vec3(0.0);
      finalColor *= exp(-fade * float(i));
      accumulatedColor += finalColor;
      lastHeight = height;
    }
    zPos += zIncrement;
  }
  
  // Final color composition
  finalColor = sqrt(clamp(accumulatedColor, 0.0, 1.0));
  finalColor += color.xyz;
  outputColor = vec4(finalColor, color.a);
}
