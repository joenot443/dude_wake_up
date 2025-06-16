#version 150

uniform float audio[256];
uniform vec2 dimensions;
uniform float time;
// uniform sampler2D tex; // tex is passed from C++ but not used in this shader. Keeping declaration commented for now.
in vec2 coord;
out vec4 outputColor;

// Main controls
uniform float audioDisplacementScale;
uniform int patternComplexity;

// New Color Uniforms
uniform float colorPaletteSeed;
uniform vec3 glowTintColor;
uniform float objectSaturation;

float detailFactor = .005;
float maxDistance = 50.0;
float pi = 3.1416;
float glowAccumulator = 0.0;
vec2 currentId;


float hash12(vec2 point)
{
  point *= 1000.0;
  vec3 p3  = fract(vec3(point.xyx) * .1031);
  p3 += dot(p3, p3.yzx + 33.33);
  return fract((p3.x + p3.y) * p3.z);
}


mat2 createRotationMatrix(float angle)
{
  float s = sin(angle);
  float c = cos(angle);
  return mat2(c, s, -s, c);
}

float getAudioValue(float position) {
  const int audioLength = 256;
  float scaledPosition = position * float(audioLength - 1);
  int index = int(scaledPosition);
  if (index < 0 || index >= audioLength - 1) {
    return 0.0;
  }
  float fraction = scaledPosition - float(index);
  float audioValue1 = audio[index];
  float audioValue2 = audio[index + 1];
  return mix(audioValue1, audioValue2, fraction);
}


float calculateBoxDistance(vec3 point, vec3 center)
{
  vec3 pointCenter = abs(point) - center;
  return length(max(vec3(0.0), pointCenter)) - min(0.0, max(pointCenter.z, max(pointCenter.x, pointCenter.y)));
}

vec2 calculateAngularMod(vec2 point, float numSegments, float offset, out float segmentIndex)
{
  float lengthFromOrigin = length(point) - offset;
  float angle = atan(point.x, point.y) / pi * numSegments * 0.5;
  segmentIndex = abs(floor(angle));
  float fractionalAngle = fract(angle) - 0.5;
  return vec2(fractionalAngle, lengthFromOrigin);
}

float calculateRingDistance(vec3 point, inout vec2 idInOut)
{
  point.xy = calculateAngularMod(point.xy * createRotationMatrix(time * 0.0), 20.0, 2.0, idInOut.x);
  
  float audioEffect = getAudioValue(0.5 + fract(idInOut.x * 0.2 + idInOut.y * 0.1)) * 0.5;
  float height = max(0.0, audioEffect * audioDisplacementScale) * 3.0 - 0.5;
  height += sin(time * 10.0 + idInOut.x) * 0.2;
  
  float distance = calculateBoxDistance(point + vec3(0.0, -height * 1.5, 0.0), vec3(0.1, height, 0.1));
  return distance * 0.5;
}

float distanceEstimator(vec3 point)
{
  float minRingDistance = 100.0;
  
  point.xz *= createRotationMatrix(time);
  point.yz *= createRotationMatrix(sin(time));
  
  float numRingSegmentsFloat = float(patternComplexity); 
  vec2 ringSegmentIds;
  for (float i = 0.0; i < numRingSegmentsFloat; i++)
  {
    point.xz *= createRotationMatrix(pi / numRingSegmentsFloat);
    ringSegmentIds.y = i;
    float currentRingDistance = calculateRingDistance(point, ringSegmentIds);
    if (currentRingDistance < minRingDistance)
    {
      minRingDistance = currentRingDistance;
      currentId = ringSegmentIds;
    }
  }
  minRingDistance = min(minRingDistance, length(point) - 1.5);
  return minRingDistance * 0.7;
}

vec3 calculateNormal(vec3 point)
{
  vec2 epsilon = vec2(0.0, detailFactor);
  return normalize(vec3(distanceEstimator(point + epsilon.yxx), distanceEstimator(point + epsilon.xyx), distanceEstimator(point + epsilon.xxy)) - distanceEstimator(point));
}


vec3 rayMarch(vec3 origin, vec3 direction)
{
  float hitDistance, totalDistance = 0.0;
  vec3 currentPosition;
  vec3 color = vec3(0.0);
  glowAccumulator = 0.0;
  for (int i = 0; i < 100; i++)
  {
    currentPosition = origin + totalDistance * direction;
    hitDistance = distanceEstimator(currentPosition);
    if (hitDistance < detailFactor || totalDistance > maxDistance) break;
    totalDistance += hitDistance;
    glowAccumulator += 0.1 / (10.0 + hitDistance * hitDistance * 10.0) * step(0.7, hash12(currentId + floor(time * 5.0)));
  }
  if (hitDistance < detailFactor)
  {
    vec3 idColor = vec3(
        hash12(currentId + colorPaletteSeed * 0.01),
        hash12(currentId + vec2(123.123, -45.67) + colorPaletteSeed * 0.015),
        hash12(currentId + vec2(-78.9, 234.56) + colorPaletteSeed * 0.005)
    );

    float luminance = dot(idColor, vec3(0.299, 0.587, 0.114));
    vec3 greyColor = vec3(luminance);
    idColor = mix(greyColor, idColor, objectSaturation);

    currentPosition -= direction * detailFactor;
    vec3 normalVector = calculateNormal(currentPosition);
    color = 0.1 + max(0.0, dot(-direction, normalVector)) * idColor;
    color *= 0.5 + step(0.7, hash12(currentId + floor(time * 5.0)));
  }
  else
  {
    direction.xz *= createRotationMatrix(time * 0.5);
    direction.yz *= createRotationMatrix(time * 0.25);
    vec2 transformedPoint = abs(0.5 - fract(direction.yz));
    float minShapeDistance = 100.0;
    float shapeIteration = 0.0;
    for(int i = 0; i < 10; i++)
    {
      transformedPoint = abs(transformedPoint * 1.3) * createRotationMatrix(radians(45.0)) - 0.5;
      float shapeDist = length(max(vec2(0.0), abs(transformedPoint) - 0.05));
      if (shapeDist < minShapeDistance)
      {
        minShapeDistance = shapeDist;
        shapeIteration = float(i);
      }
    }
    color += smoothstep(0.05, 0.0, minShapeDistance) * fract(shapeIteration * 0.1 + time) * normalize(currentPosition + 50.0);
  }
  return color * mod(gl_FragCoord.y, 4.0) * 0.5 + glowAccumulator * glowTintColor;
}



void main(  )
{
  vec2 uv = (coord - dimensions.xy * 0.5) / dimensions.y;
  vec3 rayOrigin = vec3(0.0, 0.0, -8.0);
  vec3 rayDirection = normalize(vec3(uv, 0.7));
  vec3 finalColor = rayMarch(rayOrigin, rayDirection);
  outputColor = vec4(finalColor, 1.0);
}
