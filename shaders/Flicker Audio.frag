#version 150

uniform float audio[256];
uniform vec2 dimensions;
uniform float time;
uniform int enableAudio;

in vec2 coord;
out vec4 outputColor;

const int iters = 150;

int fractal(vec2 p, vec2 point) {
  vec2 so = (-1.0 + 2.0 * point) * 0.4;
  vec2 seed = vec2(0.098386255 + so.x, 0.6387662 + so.y);
  
  for (int i = 0; i < iters; i++) {
    
    if (length(p) > 2.0) {
      return i;
    }
    vec2 r = p;
    p = vec2(p.x * p.x - p.y * p.y, 2.0* p.x * p.y);
    p = vec2(p.x * r.x - p.y * r.y + seed.x, r.x * p.y + p.x * r.y + seed.y);
  }
  
  return 0;
}

vec3 color(int i) {
  float f = float(i)/float(iters) * 2.0;
  f=f*f*2.;
  return vec3((sin(f*2.0)), (sin(f*3.0)), abs(sin(f*7.0)));
}

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

float sampleMusicA() {
  // Assuming 0.15 and 0.30 correspond to normalized positions in the audio array
  float sample0 = fft(0.0); // Sample at position 0.15 in the audio array
  float sample1 = fft(0.05); // Sample at position 0.15 in the audio array
  float sample2 = fft(0.2); // Sample at position 0.30 in the audio array
  float sample3 = fft(0.35); // Sample at position 0.30 in the audio array
  float sample4 = fft(0.5); // Sample at position 0.30 in the audio array
  float sample5 = fft(0.65); // Sample at position 0.30 in the audio array
  float sample6 = fft(0.8); // Sample at position 0.30 in the audio array
  float sample7 = fft(0.95); // Sample at position 0.30 in the audio array
  
  float total = sample0 + sample1 + sample2 + sample3 + sample4 + sample5 + sample6 + sample7;
  return total / 8.;
}

void main() {
  vec2 uv = coord.xy / dimensions.xy;
  vec2 mouse = vec2(0.5, 0.5);
  
  vec2 position = 3. * (-0.5 + coord.xy / dimensions.xy );
  position.x *= dimensions.x/dimensions.y;
  
  vec2 iFC = vec2(dimensions.x-coord.x,dimensions.y-coord.y);
  vec2 pos2 = 2. * (-0.5 + iFC.xy / dimensions.xy);
  pos2.x*=dimensions.x/dimensions.y;
  
  vec4 t3 = vec4(0.0, 0.0, 0.0, fft(position.x/2.0));
  float pulse = 0.5+sampleMusicA();
  
  vec3 invFract = color(fractal(pos2,vec2(0.55+sin(time/3.+0.5)/2.0,pulse*.9)));
  
  vec3 fract4 = color(fractal(position/1.6, vec2(0.6+cos(time/2.+0.5)/2.0,pulse*.8)));
  
  vec3 c = color(fractal(position,vec2(0.5+sin(time/3.)/2.0,pulse)));
  
  t3=abs(vec4(0.5,0.1,0.5,1.)-t3)*2.;
  
  vec4 fract01 =  vec4( c , 1.0 );
  vec4 salida;
  salida = fract01 / t3 + fract01 * t3 + vec4(invFract,0.6) + vec4(fract4,0.3);
  outputColor = salida;
}
