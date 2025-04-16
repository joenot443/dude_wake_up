#version 150

// Textures and screen information
uniform sampler2D tex; // Input texture (not used in this shader)
uniform vec2 dimensions; // Screen width and height
uniform float time; // Time in seconds

// Input from the vertex shader
in vec2 coord; // UV coordinates for the current pixel (ranging from 0 to screen dimensions)

// Output to the fragment shader
out vec4 outputColor; // Final color of the pixel (RGBA)

// Uniforms for external control
uniform vec2 mouse; // Mouse position (not used in this shader)
uniform float spectrum; // Spectrum value (used in the plasma effect)

// --- Helper Macros ---
#define time time // Redefine 'time' for clarity (no functional change)
#define resolution dimensions.xy // Screen width and height as a vec2
#define PI2 6.28318530718 // Two times Pi
#define MAX_ITER 5 // Maximum iterations for the plasma effect

// --- Snow Function ---
// Simulates falling snow
float snow(vec2 uv, float scale) {
  // Fade out snow based on vertical position
  float weight = smoothstep(1.0, 0.0, -uv.y * (scale / 10.0));
  if (weight < 0.0) return 0.0;
  
  // Scale the UV coordinates
  uv *= scale;
  // Integer and fractional parts of the scaled UVs
  vec2 gridCell = floor(uv);
  vec2 fraction = fract(uv);
  vec2 offset;
  float k = 40.0;
  float distanceToPoint;
  
  // Generate a pseudo-random offset based on the grid cell
  offset = 0.5 + 0.35 * sin(11.0 * fract(sin((gridCell + offset + scale) * mat2(7.0, 3.0, 6.0, 5.0)) * 5.0)) - fraction;
  distanceToPoint = length(offset);
  k = min(distanceToPoint, k);
  
  // Create a smooth step based on the distance and some noise
  k = smoothstep(0.0, k, sin(fraction.x + fraction.y) * 0.003);
  
  return k * weight;
}

// --- Ray Tracing Structures and Functions ---
#define MAX_DISTANCE 1.0e+30 // A very large distance for initial intersection tests

// Structure for a plane
struct Plane {
  vec3 origin;
  vec3 normal;
  vec3 color;
};

// Structure for a sphere
struct Sphere {
  float radius;
  vec3 origin;
  vec3 color;
};

// Structure for an Axis-Aligned Bounding Box (AABB)
struct AABox {
  vec3 min;
  vec3 max;
  vec3 color;
};

// Structure to store intersection information
struct Intersection {
  float t; // Distance along the ray to the intersection point
  vec3 point; // 3D coordinates of the intersection point
  vec3 normal; // Surface normal at the intersection point
  vec3 color; // Color of the intersected object
};

// Calculates the normal vector at a point on an AABB
vec3 normalAtPointOnAABB(AABox aabox, vec3 point) {
  float epsilon = -0.001;
  vec3 halfSize = (aabox.max - aabox.min) * 0.5;
  vec3 center = (aabox.min + aabox.max) * 0.5;
  vec3 vectorToPoint = point - center;
  return normalize(sign(vectorToPoint) * step(epsilon, abs(vectorToPoint) - halfSize));
}

// Intersects a ray with an AABB
Intersection intersectAABB(vec3 rayOrigin, vec3 rayDirection, AABox aabox, Intersection info) {
  vec3 inverseDirection = 1.0 / rayDirection;
  vec3 nearIntersection = (aabox.min - rayOrigin) * inverseDirection;
  vec3 farIntersection = (aabox.max - rayOrigin) * inverseDirection;
  vec3 minValues = min(nearIntersection, farIntersection);
  vec3 maxValues = max(nearIntersection, farIntersection);
  float tNear = max(minValues.x, max(minValues.y, minValues.z));
  float tFar = min(maxValues.x, min(maxValues.y, maxValues.z));
  
  if (tFar < 0.0) return info; // Box is behind the ray
  if (tNear > tFar) return info; // Ray does not intersect the box
  
  float t = tNear;
  if ((t > 0.0) && (t < info.t)) {
    info.t = t;
    info.point = rayOrigin + rayDirection * t;
    info.normal = normalAtPointOnAABB(aabox, info.point);
    info.color = aabox.color;
  }
  return info;
}

// Intersects a ray with a sphere
Intersection intersectSphere(vec3 rayOrigin, vec3 rayDirection, Sphere sphere, Intersection info) {
  vec3 originToRay = sphere.origin - rayOrigin;
  float b = dot(originToRay, rayDirection);
  float discriminant = b * b - dot(originToRay, originToRay) + sphere.radius * sphere.radius;
  if (discriminant < 0.0) return info;
  float root = sqrt(discriminant);
  float t = b - root; // First intersection point
  
  if ((t > 0.0) && (t < info.t)) {
    info.t = t;
    info.point = rayOrigin + rayDirection * t;
    info.normal = normalize(info.point - sphere.origin);
    info.color = sphere.color;
  }
  return info;
}

// Intersects a ray with a plane
Intersection intersectPlane(vec3 rayOrigin, vec3 rayDirection, Plane plane, Intersection info) {
  float normalDotDirection = dot(rayDirection, plane.normal);
  if (normalDotDirection >= 0.0) return info; // Ray is parallel or pointing away
  
  float originDotNormal = dot(plane.origin, plane.normal);
  float t = -(dot(rayOrigin, plane.normal) - originDotNormal) / normalDotDirection;
  
  if ((t > 0.0) && (t < info.t)) {
    info.t = t;
    info.point = rayOrigin + rayDirection * t;
    info.normal = plane.normal;
    // Checkerboard pattern on the plane
    float gridValue = abs(floor(info.point.x) + floor(info.point.z));
    info.color = plane.color * (mod(gridValue, 2.0) < 1.0 ? 0.5 : 1.0);
  }
  return info;
}

// Intersects the ray with all objects in the scene
Intersection intersectScene(vec3 rayOrigin, vec3 rayDirection) {
  Intersection info;
  info.t = MAX_DISTANCE;
  info.color = vec3(0.0, 0.0, 0.0); // Initialize with black background
  
  // Define the plane
  Plane plane = Plane(
                      vec3(0.0, 0.0, 0.0), // origin
                      vec3(0.0, 1.0, 0.0), // normal (pointing upwards)
                      vec3(1.0, 1.0, 1.0)  // color (white)
                      );
  info = intersectPlane(rayOrigin, rayDirection, plane, info);
  
  // Define the sphere
  Sphere sphere = Sphere(
                         0.5, // radius
                         vec3(0.0, 0.5, 0.0), // origin
                         vec3(1.0, 1.0, 1.0)  // color (white)
                         );
  info = intersectSphere(rayOrigin, rayDirection, sphere, info);
  
  // Define the AABB
  AABox aabox = AABox(
                      vec3(-1.5, 0.0, 0.5), // min corner
                      vec3(-0.5, 2.0, 1.5),  // max corner
                      vec3(1.0, 1.0, 1.0)   // color (white)
                      );
  info = intersectAABB(rayOrigin, rayDirection, aabox, info);
  
  return info;
}

// --- Matrix Transformation Functions ---
// Creates a translation matrix
mat4 translate(vec3 t) {
  return mat4(
              vec4(1.0, 0.0, 0.0, 0.0),
              vec4(0.0, 1.0, 0.0, 0.0),
              vec4(0.0, 0.0, 1.0, 0.0),
              vec4(t.x, t.y, t.z, 1.0)
              );
}

// Creates a scaling matrix
mat4 scale(vec3 s) {
  return mat4(
              vec4(s.x, 0.0, 0.0, 0.0),
              vec4(0.0, s.y, 0.0, 0.0),
              vec4(0.0, 0.0, s.z, 0.0),
              vec4(0.0, 0.0, 0.0, 1.0)
              );
}

// Creates an orthographic projection matrix
mat4 ortho(float left, float right, float bottom, float top, float near, float far) {
  return scale(vec3(2.0 / (right - left), 2.0 / (top - bottom), 2.0 / (far - near))) *
  translate(vec3(-(left + right) / 2.0, -(top + bottom) / 2.0, -(far + near) / 2.0));
}

// Creates a projection matrix (used internally by perspective)
mat4 projection(float near, float far) {
  return mat4(
              vec4(near, 0.0, 0.0, 0.0),
              vec4(0.0, near, 0.0, 0.0),
              vec4(0.0, 0.0, near + far, 1.0),
              vec4(0.0, 0.0, -far * near, 0.0)
              );
}

// Creates a perspective projection matrix
mat4 perspective(float fovRadians, float aspect, float near, float far) {
  float top = tan(fovRadians * 0.5) * near;
  float bottom = -top / aspect;
  float left = -top;
  float right = top / aspect;
  return ortho(left, right, bottom, top, near, far) * projection(near, far) * scale(vec3(1.0, 1.0, -1.0));
}

// Creates a view matrix (camera transformation)
mat4 lookAt(vec3 eye, vec3 center, vec3 up) {
  vec3 z = normalize(eye - center);
  vec3 x = normalize(cross(up, z));
  vec3 y = normalize(cross(z, x));
  mat4 rotation = mat4(
                       vec4(x.x, y.x, z.x, 0.0),
                       vec4(x.y, y.y, z.y, 0.0),
                       vec4(x.z, y.z, z.z, 0.0),
                       vec4(0.0, 0.0, 0.0, 1.0)
                       );
  return rotation * translate(-eye);
}

// --- 2D Rotation Matrix ---
#define PI 3.141592
#define TWOPI 6.283184
#define R2D 180.0 / PI *
#define D2R PI / 180.0 *

mat2 rotMat(in float angleRadians) {
  float cosAngle = cos(angleRadians);
  float sinAngle = sin(angleRadians);
  return mat2(cosAngle, -sinAngle, sinAngle, cosAngle);
}

// --- Absolute Looping Functions ---
// Absolute value of a 1D looping coordinate
float abs1d(in float x) {
  return abs(fract(x) - 0.5);
}

// Absolute value of a 2D looping coordinate
vec2 abs2d(in vec2 v) {
  return abs(fract(v) - 0.5);
}

// --- Oil Noise Function ---
#define OC 15.0 // Number of octaves for the noise

vec3 Oilnoise(in vec2 position, in vec3 baseColor) {
  vec2 q = vec2(0.0);
  float result = 0.0;
  
  float scale = 2.2;
  float gain = 0.44;
  vec2 absolutePosition = abs2d(position) * 0.5; // Add position
  
  for (float i = 0.0; i < OC; i++) {
    position *= rotMat(D2R 30.0); // Rotate the position
    float timeOffset = (sin(time) * 0.015 + 0.5) * 0.2 + time * 0.08;
    q = position * scale + absolutePosition + timeOffset;
    q = vec2(cos(q));
    
    result += abs1d(dot(q, vec2(0.3))) * gain;
    
    scale *= 1.07;
    absolutePosition += cos(q);
    absolutePosition *= rotMat(D2R 5.0);
    absolutePosition *= 1.2;
  }
  
  result = pow(result, 4.0);
  return clamp(baseColor / result, vec3(0.0), vec3(1.0));
}

void main() {
  // Normalize pixel coordinates to the range [-0.5, 0.5]
  vec2 normalizedPosition = (gl_FragCoord.xy / resolution.xy) - 0.5;
  
  // Normalized UV coordinates centered and scaled based on aspect ratio
  vec2 uv = (gl_FragCoord.xy * 2.0 - resolution.xy) / min(resolution.x, resolution.y);
  vec3 finalColor = vec3(0.0);
  
  // --- Lighting Setup ---
  float lightTime = time * 0.12;
  float lightIntensity = 10.0;
  float lightSpeed = 1.0 * lightTime;
  float lightAngle = mod(lightSpeed, TWOPI);
  vec3 lightPosition = vec3(
                            8.0 * sin(lightAngle),
                            2.0,
                            8.0 * cos(lightAngle)
                            );
  
  // --- 3D Scene Rendering ---
  float aspectRatio = dimensions.x / dimensions.y;
  mat4 projectionMatrix = perspective(radians(90.0), aspectRatio, 0.001, 1000.0);
  
  vec3 oilNoiseColor = vec3(0.0);
  vec2 screenUV = (coord / dimensions.xy);
  screenUV.x = ((screenUV.x - 0.5) * aspectRatio) + 0.5;
  float screenMask = step(0.0, screenUV.x * (1.0 - screenUV.x));
  
  // Apply oil noise
  vec2 noiseUV = screenUV * 3.0;
  vec3 baseRGB = vec3(1.30, 0.8, 1.200);
  
  // Anti-aliasing for oil noise
  float antiAliasingSamples = 1.0;
  vec2 pixelSize = 1.0 / dimensions.xy;
  vec2 antiAliasedUV = vec2(0.0);
  
  for (float i = 0.0; i < antiAliasingSamples; i++) {
    for (float j = 0.0; j < antiAliasingSamples; j++) {
      antiAliasedUV = noiseUV + pixelSize * vec2((i + 0.5) / antiAliasingSamples, (j + 0.5) / antiAliasingSamples);
      oilNoiseColor += Oilnoise(antiAliasedUV, baseRGB);
    }
  }
  oilNoiseColor /= (antiAliasingSamples * antiAliasingSamples);
  
  // Camera setup
  vec3 cameraOrigin = vec3(2.0, 2.0, 15.0);
  vec3 cameraTarget = vec3(0.0, 0.0, 0.0);
  mat4 viewMatrix = lookAt(cameraOrigin, cameraTarget, vec3(0.0, 1.0, 0.0));
  
  // Ray direction calculation
  vec2 normalizedDeviceCoords = coord.xy / dimensions.xy * 2.0 - 1.0; // [-1, 1]
  vec4 viewDirection = inverse(projectionMatrix) * vec4(normalizedDeviceCoords, -1.0, 1.0);
  viewDirection.w = 0.0; // Set w to 0 for a direction vector
  vec3 rayDirection = normalize((inverse(viewMatrix) * viewDirection).xyz);
  
  // Perform ray tracing
  Intersection intersectionInfo = intersectScene(cameraOrigin, rayDirection);
  
  // Apply lighting if there was an intersection
  if (intersectionInfo.t < MAX_DISTANCE) {
    // Shadow calculation
    vec3 shadowOrigin = intersectionInfo.point;
    vec3 shadowDirection = lightPosition - shadowOrigin;
    float shadowDistanceSquared = dot(shadowDirection, shadowDirection);
    float shadowDistance = sqrt(shadowDistanceSquared);
    shadowDirection /= shadowDistance; // Normalize
    
    Intersection shadowIntersectionInfo = intersectScene(shadowOrigin, shadowDirection);
    
    // Determine if the light is blocked
    float directLighting = (shadowIntersectionInfo.t > shadowDistance) ? 1.0 : 0.0;
    float lambertianFactor = max(0.0, dot(intersectionInfo.normal, shadowDirection));
    float attenuation = lightIntensity / shadowDistanceSquared;
    float shading = min(1.0, directLighting * lambertianFactor * attenuation);
    
    vec3 shadedColor = intersectionInfo.color * shading;
    vec3 toneMappedColor = pow(shadedColor, vec3(1.0 / 2.2)); // Gamma correction
    
    intersectionInfo.color = toneMappedColor;
  }
  
  // --- Render the Sun ---
  vec4 sunWorldPosition = vec4(lightPosition, 1.0);
  vec4 sunViewPosition = viewMatrix * sunWorldPosition;
  vec4 sunProjectedPosition = projectionMatrix * sunViewPosition;
  vec3 sunScreenPosition = sunProjectedPosition.xyz / sunProjectedPosition.w;
  sunScreenPosition.xy = (sunScreenPosition.xy + 1.0) * 0.5;
  sunScreenPosition.xy = sunScreenPosition.xy * dimensions.xy - 1.0;
  
  float sunSize = dimensions.y * 3.25; // Scale by screen height
  sunSize /= distance(lightPosition, cameraOrigin); // Scale by distance to camera
  float distanceToSun = distance(sunScreenPosition.xy, coord.xy);
  vec3 sunColor = vec3(1.0, 0.5, 2.1) * sunSize / distanceToSun;
  vec3 tonedSunColor = pow(sunColor, vec3(1.0 / 2.2)); // Gamma correction
  
  // Combine ray traced color and sun color
  intersectionInfo.color = min((intersectionInfo.color + tonedSunColor) * 0.5, vec3(1.0));
  
  // --- Plasma Effect ---
  vec2 plasmaUV = mod(uv * PI2, PI2) - 254.0;
  vec2 plasmaIterator = vec2(plasmaUV);
  float plasmaValue = 1.2;
  float intensity = 0.0064;
  
  for (int n = 0; n < MAX_ITER; n++) {
    float timeOffset = time * (1.0 - (3.2 / float(n + 1)));
    plasmaIterator = plasmaUV + vec2(cos(timeOffset - plasmaIterator.x) + sin(timeOffset + plasmaIterator.y),
                                     sin(timeOffset - plasmaIterator.y) + cos(timeOffset + plasmaIterator.x));
    plasmaValue += 1.0 / length(vec2(plasmaUV.x / (sin(plasmaIterator.x + timeOffset) / intensity + spectrum),
                                     plasmaUV.y / (cos(plasmaIterator.y + timeOffset) / intensity)));
  }
  
  plasmaValue /= float(MAX_ITER);
  plasmaValue = 0.22 - pow(plasmaValue, 1.22);
  vec3 plasmaColor = vec3(0.098, 0.098, 0.098 + pow(abs(plasmaValue), 5.2));
  float snowAmount = smoothstep(0.1, 0.0, clamp(uv.x * 0.11 + 0.99, 0.0, 0.99));
  snowAmount += snow(uv, 3.0) * 0.8;
  snowAmount += snow(uv, 5.0) * 0.7;
  snowAmount += snow(uv, 7.0) * 0.6;
  snowAmount += snow(uv, 9.0) * 0.5;
  snowAmount += snow(uv, 11.0) * 0.4;
  snowAmount += snow(uv, 13.0) * 0.3;
  snowAmount += snow(uv, 15.0) * 0.2;
  snowAmount += snow(uv, 17.0) * 0.1;
  
  // --- Orbiting Light ---
  vec2 orbitCenter = dimensions.xy * 0.5;
  vec2 orbitRadius = dimensions.yy * 0.4;
  float orbitSpeed = time * 0.1;
  float orbitAngle = mod(orbitSpeed * TWOPI, TWOPI);
  vec2 orbitPositionOffset = orbitRadius * vec2(sin(orbitAngle), cos(orbitAngle));
  vec2 orbitingLightOrigin = orbitPositionOffset + orbitCenter;
  float orbitingLightSize = dimensions.y * 0.1;
  float distanceToOrbitingLight = length(coord - orbitingLightOrigin);
  vec3 orbitingLightColor = vec3(1.0, 1.0, 1.0) * orbitingLightSize / distanceToOrbitingLight;
  
  // --- Radial Color Effect ---
  vec2 centerOffset = -1.0 + 2.0 * coord.xy / dimensions.xy;
  float centerDistance = length(centerOffset);
  vec2 distortedUV = uv + (centerOffset / centerDistance) * cos(centerDistance * 12.0 - time * 10.0) * 0.03;
  
  // --- Animated Shapes ---
  vec2 p = gl_FragCoord.xy / resolution.xy;
  vec2 q = p - vec2(0.5, 0.5);
  vec3 animatedColor = 0.5 + 0.5 * sin(vec3(0, 2, 4) + time / 2.0);
  float radius1 = 0.4 + 0.1 * cos(atan(-p.x + 2.0, q.y / 2.0) * 60.0 + 20.0 * -q.x + ((length(mouse / 0.4) / 0.25) * 2.0));
  float radius2 = 0.2 + 0.1 * cos(atan(-p.x + 2.0, -p.y / 2.0) * 5.0 + 20.0 * -q.x + time * 0.5);
  float radius3 = 0.4 + 0.1 * cos(atan(-p.x + 4.0, -q.y / 2.0) * 10.0 + 20.0 * -q.x + time * 0.5);
  float radius4 = 0.6 + 0.1 * cos(atan(-p.x + 6.0, -q.y / 2.0) * 20.0 + 20.0 * -q.x + time * 0.5);
  
  animatedColor /= smoothstep(radius1 / 100.0, radius1, length(q)) / 0.2;
  animatedColor.x /= smoothstep(radius2 / 100.0, radius2, length(q)) / 0.2;
  animatedColor.y /= smoothstep(radius3 / 100.0, radius3, length(q)) / 0.2;
  animatedColor.z /= smoothstep(radius4 / 100.0, radius4, length(q)) / 0.2;
  
  // --- Wavy Lines ---
  float wavyLineY = 0.2 * normalizedPosition.y * sin(200.0 * normalizedPosition.y - 20.0 * time * 1.35);
  vec3 wavyLineColor = vec3(wavyLineY * 5.0, wavyLineY, wavyLineY * 5.0);
  float sharpLine1 = 1.0 / (600.0 * abs(normalizedPosition.y - wavyLineY));
  float sharpLine2 = 1.0 / (665.0 * length(normalizedPosition - vec2(0.0, normalizedPosition.y)));
  float centralPoint = 1.0 / (65.0 * length(normalizedPosition - vec2(0.0, 0.0)));
  
  // --- Final Color Output ---
  outputColor = vec4(vec3(snowAmount * 10.0, snowAmount * 5.1, snowAmount * 01.9) + intersectionInfo.color + orbitingLightColor * 0.1, 1.0);
  outputColor += vec4(finalColor * 10.0 + plasmaColor * 0.25 * oilNoiseColor * 6.5, 1.0);
  outputColor += vec4(animatedColor, 1.0);
  outputColor += vec4(wavyLineColor * sharpLine1 * 0.1 + wavyLineColor * sharpLine2 * 0.1 + vec3(centralPoint * 0.1), 1.0);
}
