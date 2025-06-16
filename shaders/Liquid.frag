#version 150

// Input uniforms
uniform sampler2D tex;
uniform vec2 dimensions;
uniform float time;
uniform float intensity;
uniform float flowSpeed;
uniform float turbulence;

in vec2 coord;
out vec4 outputColor;

// Simplex noise implementation
lowp vec3 permute(in lowp vec3 x) { return mod(x * x * 34. + x, 289.); }

lowp float snoise(in lowp vec2 v) {
  lowp vec2 cell = floor((v.x + v.y) * .36602540378443 + v),
            offset0 = (cell.x + cell.y) * .211324865405187 + v - cell;
  lowp float step = step(offset0.x, offset0.y);
  lowp vec2 offset1 = vec2(1.0 - step, step),
            offset2 = offset0 - offset1 + .211324865405187,
            offset3 = offset0 - .577350269189626;
  cell = mod(cell, 289.);
  lowp vec3 perm = permute(permute(cell.y + vec3(0, offset1.y, 1)) + cell.x + vec3(0, offset1.x, 1)),
            weights = max(.5 - vec3(dot(offset0, offset0), dot(offset2, offset2), dot(offset3, offset3)), 0.),
            noise = fract(perm * .024390243902439) * 2. - 1.,
            absNoise = abs(noise) - .5,
            noiseOffset = noise - floor(noise + .5);
  return .5 + 65. * dot(pow(weights, vec3(4.)) * (-0.85373472095314 * (noiseOffset * noiseOffset + absNoise * absNoise) + 1.79284291400159),
                        noiseOffset * vec3(offset0.x, offset2.x, offset3.x) + absNoise * vec3(offset0.y, offset2.y, offset3.y));
}

void main() {
  // Normalize UV coordinates
  vec2 uv = coord.xy / dimensions.xy;
  uv /= 1.1;
  uv += .01;
  uv *= 1.;
  
  // Time-based animation with flow speed
  float timeScaled = time * flowSpeed;
  float edgeBlend = smoothstep(0., 1., uv.x);
  
  // Enhanced turbulence effect
  float turbFactor = turbulence * (1.0 + sin(timeScaled * 0.5) * 0.2);
  
  // Apply vertical sine wave distortion with turbulence
  float verticalDistortion = sin(timeScaled + uv.x * 5.) * (.05 * intensity);
  uv.y += edgeBlend * verticalDistortion * turbFactor;
  
  // Apply horizontal noise-based distortion with enhanced turbulence
  float noiseScale = 4.3 * (edgeBlend / 3.7 + 1.2) * turbFactor;
  float noiseOffset = snoise(uv * noiseScale - vec2(timeScaled * 1.2, 0.)) * intensity;
  uv.x += edgeBlend * noiseOffset;
  
  // Sample and output the distorted texture
  outputColor = texture(tex, uv);
}
