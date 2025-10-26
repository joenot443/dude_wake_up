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
  int tx = int(uv.x * 255.0);

  // Get actual waveform data (time-domain audio samples)
  float wave = audio[tx];

  // Calculate distance from waveform line
  float dist = abs(wave - (uv.y - 0.5) * 2.0);

  // Base waveform line
  float baseLine = 1.0 - smoothstep(0.0, 0.01, dist);

  // Outline - slightly thicker region around the base line
  float outlineWidth = 0.01 + (outlineThickness * 0.01);
  float outline = 1.0 - smoothstep(0.01, outlineWidth, dist);

  // Combine colors: outline where there's no base, otherwise use base
  vec3 finalColor = mix(outlineColor, baseColor, baseLine);
  float alpha = max(baseLine, outline);

  // output final color
  outputColor = vec4(finalColor * alpha, alpha);
}
