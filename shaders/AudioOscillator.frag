#version 150

uniform sampler2D tex;
uniform vec2 dimensions;
uniform float time;
uniform float audio[256];
uniform vec3 baseColor;
uniform vec3 outlineColor;
uniform float outlineThickness;
in vec2 coord;
out vec4 outputColor;

// Created by inigo quilez - iq/2013
// https://www.youtube.com/c/InigoQuilez
// https://iquilezles.org/


// See also:
//
// Input - Keyboard    : https://www.shadertoy.com/view/lsXGzf
// Input - Microphone  : https://www.shadertoy.com/view/llSGDh
// Input - Mouse       : https://www.shadertoy.com/view/Mss3zH
// Input - Sound       : https://www.shadertoy.com/view/Xds3Rr
// Input - SoundCloud  : https://www.shadertoy.com/view/MsdGzn
// Input - Time        : https://www.shadertoy.com/view/lsXGz8
// Input - TimeDelta   : https://www.shadertoy.com/view/lsKGWWV
// Inout - 3D Texture  : https://www.shadertoy.com/view/4llcR4


void main(  )
{
  // create pixel coordinates
  vec2 uv = coord.xy / dimensions.xy;

  // Map to our audio array (256 samples)
  float xIndex = uv.x * 255.0;
  int tx = int(xIndex);
  int txNext = min(255, tx + 1);

  // Get actual waveform data (time-domain audio samples)
  float wave = audio[tx];
  float waveNext = audio[txNext];

  // Center line at y = 0.5 (matching AudioSourceBrowserView style)
  float centerLineDist = abs(uv.y - 0.5);
  float centerLine = 1.0 - smoothstep(0.0, 0.0015, centerLineDist);
  vec3 centerLineColor = vec3(80.0 / 255.0, 80.0 / 255.0, 80.0 / 255.0);

  // Map waveform similar to AudioSourceBrowserView: center_y - wave * scale * height * 0.4
  // Waveform is centered at 0.5, scaled by 0.4 (matching AudioSourceBrowserView)
  float scale = 1.0; // Fixed scale (AudioSourceBrowserView uses frequencyScale parameter)
  float waveY = 0.5 - wave * scale * 0.4;
  float waveYNext = 0.5 - waveNext * scale * 0.4;

  // Calculate distance from waveform line segment (between current and next point)
  // This matches how AudioSourceBrowserView draws lines between consecutive points
  float dist = 0.0;
  if (tx < 255) {
    // Interpolate waveform position along x-axis to create line segment
    float t = fract(xIndex);
    float waveYInterp = mix(waveY, waveYNext, t);
    dist = abs(uv.y - waveYInterp);
    
    // Also check distance perpendicular to the line segment for proper line rendering
    vec2 p1 = vec2(float(tx) / 255.0, waveY);
    vec2 p2 = vec2(float(txNext) / 255.0, waveYNext);
    vec2 p = vec2(uv.x, uv.y);
    
    // Calculate distance from point to line segment
    vec2 lineDir = p2 - p1;
    float lineLen = length(lineDir);
    if (lineLen > 0.0) {
      vec2 lineNorm = normalize(lineDir);
      vec2 toPoint = p - p1;
      float proj = dot(toPoint, lineNorm);
      proj = clamp(proj, 0.0, lineLen);
      vec2 closestPoint = p1 + lineNorm * proj;
      dist = length(p - closestPoint);
    }
  } else {
    dist = abs(uv.y - waveY);
  }

  // Thinner waveform line (matching 2.0f pixel thickness from AudioSourceBrowserView)
  // Using a smaller smoothstep range for thinner, crisper line
  float lineThickness = 0.002 + (outlineThickness * 0.0005);
  float waveformLine = 1.0 - smoothstep(0.0, lineThickness, dist);

  // Use waveform color similar to AudioSourceBrowserView: RGB(100, 200, 255)
  vec3 waveformColor = vec3(100.0 / 255.0, 200.0 / 255.0, 255.0 / 255.0);
  
  // Use baseColor if it's been customized (not default white), otherwise use waveform color
  float isDefaultWhite = step(0.99, baseColor.r) * step(0.99, baseColor.g) * step(0.99, baseColor.b);
  vec3 finalWaveformColor = mix(waveformColor, baseColor, 1.0 - isDefaultWhite);

  // Combine center line and waveform (waveform takes priority)
  vec3 finalColor = mix(centerLineColor, finalWaveformColor, waveformLine);
  float alpha = max(centerLine * 0.5, waveformLine); // Center line is semi-transparent

  // output final color
  outputColor = vec4(finalColor * alpha, alpha);
}
