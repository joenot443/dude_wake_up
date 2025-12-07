//
//  WaveformOscillator.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 12/26/22.
//

#include "WaveformOscillator.hpp"
#include "Video.hpp"

const float OscillatorDampen = 0.3;

// Constructor with custom values - converts old amplitude/shift to min/max
WaveformOscillator::WaveformOscillator(std::shared_ptr<Parameter> param, float amplitude, float shift, float frequency)
  : WaveformOscillator(param)
{
  // Convert old amplitude/shift to min/max output
  float halfRange = 0.3 * amplitude;  // OscillatorDampen * amplitude
  this->minOutput->setValue(shift - halfRange);
  this->maxOutput->setValue(shift + halfRange);
  this->frequency->setValue(frequency);
}

void WaveformOscillator::tick()
{
  if (!enabled->boolValue)
    return;

  float t = frameTime();
  // Use a consistent phase-shifted time
  float freq_t = frequency->value * t + randOffset; 
  float v = 0.0f;

  // Determine the waveform based on the waveShape parameter
  switch (static_cast<WaveShape>(waveShape->intValue)) {
    case Sine:
      v = sin(freq_t); // Use freq_t for consistency
      break;
    case Square:
      v = (sin(freq_t) >= 0) ? 1.0f : -1.0f; // Use freq_t
      break;
    case Sawtooth:
      // Simpler: Use fmod based on freq_t
      v = 2.0f * fmod(freq_t / TWO_PI, 1.0f) - 1.0f; // Sawtooth from 0 to 1, scaled to -1 to 1
      break;
    case Triangle:
       // Simpler: Based on Sawtooth
       v = 2.0f * fabs(2.0f * fmod(freq_t / TWO_PI, 1.0f) - 1.0f) - 1.0f; // Triangle wave based on fmod
      break;
      
    // --- New Waveforms ---
    case PulseTrain:
    {
      // Simple pulse train with 10% duty cycle
      float dutyCycle = 0.1f; 
      v = (fmod(freq_t / TWO_PI, 1.0f) < dutyCycle) ? 1.0f : -1.0f;
      break;
    }
    case ExponentialSine:
      // Note: Range is approx [-0.37, 2.72], amplitude behaviour will differ.
      v = sin(freq_t) * exp(sin(freq_t)); 
      break;
    case HarmonicStack:
    {
      // Sum of first 3 harmonics, normalized
      float norm = 1.0f + 0.5f + 0.25f; 
      v = (sin(freq_t) + 0.5f * sin(2.0f * freq_t) + 0.25f * sin(3.0f * freq_t)) / norm;
      break;
    }
    case RectifiedSine:
       // Scaled absolute value of sine to fit [-1, 1]
      v = 2.0f * fabs(sin(freq_t)) - 1.0f;
      break;
    case NoiseModulatedSine:
    {
       // Sine wave frequency modulated by Perlin noise
       // Noise parameters (time scale, amplitude) might become parameters later
      float noiseTimeScale = 0.1f; 
      float noiseAmplitude = 5.0f;
      v = sin(freq_t + ofNoise(t * noiseTimeScale) * noiseAmplitude);
      break;
    }
    case BitcrushedSine:
    {
      // Quantize sine wave output
      // Step count might become a parameter later
      float steps = 8.0;
      float sine_val = sin(freq_t);
      v = floor(sine_val * steps) / steps;
      // Scale to ensure it reaches full range potentially
      v = v * (steps / (steps - 1.0)); // Optional scaling correction
      v = fmin(1.0, fmax(-1.0, v)); // Clamp just in case
      break;
    }
    case MoireOscillation:
    {
      // Combine two sine waves with slightly different frequencies
      // Frequency ratio might become a parameter later
      float freqRatio = 1.05f; 
      float freq2_t = (frequency->value * freqRatio) * t + randOffset;
      // Mix them equally
      v = 0.5f * sin(freq_t) + 0.5f * sin(freq2_t); 
      break;
    }
  }

  // Map waveform from [-1, 1] to [minOutput, maxOutput]
  // First normalize to [0, 1]
  float normalized = (v + 1.0f) / 2.0f;

  // Ensure minOutput <= maxOutput
  float actualMin = fmin(minOutput->value, maxOutput->value);
  float actualMax = fmax(minOutput->value, maxOutput->value);

  // Map to output range
  float outputValue = actualMin + normalized * (actualMax - actualMin);

  // Clamp to parameter bounds (shouldn't be necessary, but safety check)
  outputValue = fmin(fmax(outputValue, value->min), value->max);

  value->setValue(outputValue);

  // Maintain a fixed number of data points
  if (data.size() >= 1000) {
    data.erase(data.begin()); // Remove the oldest data point
  }
  // Add the *final* parameter value to the data log
  data.push_back(ImVec2(t, value->value)); 
}

float WaveformOscillator::max()
{
  return fmax(minOutput->value, maxOutput->value);
}

float WaveformOscillator::min()
{
  return fmin(minOutput->value, maxOutput->value);
}
