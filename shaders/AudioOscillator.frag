#version 150

uniform sampler2D tex;
uniform vec2 dimensions;
uniform float time;
uniform float audio[256];
uniform float waveform[256];
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

  // Get frequency data from audio array (FFT)
  float fft = audio[tx];

  // Get actual waveform data (time-domain audio samples)
  float wave = waveform[tx];

  // add wave form on top - now using actual waveform data
  float col = 1.0 - smoothstep( 0.0, 0.15, abs(wave - (uv.y - 0.5) * 2.0) );

  // output final color
  outputColor = vec4(col, col, col, 1.0);
}
