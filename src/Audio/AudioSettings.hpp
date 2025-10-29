//
//  AudioSettings.h
//  dude_wake_up
//
//  Created by Joe Crozier on 2022-05-30.
//

#ifndef AudioSettings_h
#define AudioSettings_h

#include <cmath>
#include <chrono>
#include <iostream>
#include <deque>
#include "Gist.h"
#include "Math.hpp"
#include "Oscillator.hpp"
#include "Parameter.hpp"
#include "Pulser.hpp"
#include "PulseOscillator.hpp"
#include "ValueOscillator.hpp"
#include "Vectors.hpp"
#include "RollingMean.hpp" // Include the new RollingMean class

enum BpmMode {
  BpmMode_Auto,
  BpmMode_Manual,
  BpmMode_Link
};

enum SmoothingMode {
  SmoothingMode_Exponential,
  SmoothingMode_MovingAverage,
  SmoothingMode_None,
  SmoothingMode_PeakHold
};

struct AudioAnalysisParameter {
  float windowMin = 0.0;
  float windowMax = 0.0;

  float value = 0.0;
  float rollingMean = 0.0;
  float smoothedValue = 0.0;

  // Vary from 0.0 to 1.0: rollingMean / windowMax:
  float rollingMeanRelation = 0.0;

  Pulser pulser;

  std::shared_ptr<Parameter> pulseLength;
  std::shared_ptr<Parameter> pulseThreshold;
  std::shared_ptr<Parameter> release;
  std::shared_ptr<Parameter> param;
  std::shared_ptr<Parameter> pulse;
  std::shared_ptr<ValueOscillator> pulseOscillator;
  std::shared_ptr<ValueOscillator> thresholdOscillator;

  RollingMean valueAcc;   // Use RollingMean instead of Boost Accumulator
  std::deque<double> minMaxValues;  // Store values to manually track min and max
  size_t minMaxWindowSize = 800;

  AudioAnalysisParameter(std::shared_ptr<Parameter> param) :
  valueAcc(2),   // Rolling window size for mean
  pulseLength(std::make_shared<Parameter>("pulseLength", 0.8, 0.0, 1.0)),
  pulseThreshold(std::make_shared<Parameter>("pulseThreshold", 0.6, 0.0, 1.0)),
  release(std::make_shared<Parameter>("Release", 0.6, 0.0, 1.0)),
  pulse(std::make_shared<Parameter>("autoPulse", 0.0, 0.0, 1.0)),
  pulseOscillator(std::make_shared<ValueOscillator>(pulse)),
  thresholdOscillator(std::make_shared<ValueOscillator>(pulseThreshold)),
  pulser(pulseThreshold, pulseLength),
  param(param) {
    pulser = Pulser(pulseThreshold, pulseLength);
  }

  void tick(float val) {
    value = val;

    // Apply exponential smoothing to the raw value first
    smoothedValue = smoothedValue * release->value + val * (1.0f - release->value);

    valueAcc.add(smoothedValue);  // Add smoothed value to RollingMean accumulator

    // Update the min/max deque with smoothed values
    minMaxValues.push_back(smoothedValue);
    if (minMaxValues.size() > minMaxWindowSize) {
        minMaxValues.pop_front();
    }

    // Calculate min and max manually
    windowMin = *std::min_element(minMaxValues.begin(), minMaxValues.end());
    windowMax = *std::max_element(minMaxValues.begin(), minMaxValues.end());

    rollingMean = valueAcc.mean();  // Get rolling mean
    rollingMeanRelation = relationToRange(rollingMean);

    param->value = rollingMeanRelation;
    pulse->value = rollingMeanRelation;
  }

  float relationToRange(float v) {
    float range = windowMax - windowMin;
    return (range == 0.0) ? 0.0 : v / range;  // Handle division by zero
  }
};

struct AudioAnalysis {
  std::string name;
  bool bpmEnabled = true;
  BpmMode bpmMode = BpmMode_Auto;
  SmoothingMode smoothingMode = SmoothingMode_MovingAverage;
  std::shared_ptr<Parameter> rms;
  std::shared_ptr<Parameter> highs;
  std::shared_ptr<Parameter> lows;
  std::shared_ptr<Parameter> mids;
  std::shared_ptr<Parameter> beatPulse;
  std::shared_ptr<Parameter> beatOscillatorSelection;
  std::shared_ptr<Parameter> bpm;
  std::shared_ptr<Parameter> bpmModeParam;
  std::shared_ptr<Parameter> bpmLocked;
  std::shared_ptr<Parameter> bpmNudge;
  std::shared_ptr<Parameter> frequencyRelease;
  std::shared_ptr<Parameter> frequencyScale;

  std::shared_ptr<ValueOscillator> rmsOscillator;
  std::shared_ptr<PulseOscillator> beatPulseOscillator;
  std::shared_ptr<ValueOscillator> highsOscillator;
  std::shared_ptr<ValueOscillator> midsOscillator;
  std::shared_ptr<ValueOscillator> lowsOscillator;

  std::vector<float> melFrequencySpectrum;
  std::vector<float> magnitudeSpectrum;
  std::vector<float> smoothSpectrum;
  std::vector<float> smoothMelSpectrum;
  std::vector<float> waveform;
  std::vector<float> smoothWaveform;

  // Smoothing buffers for different modes
  std::deque<std::vector<float>> spectrumHistory;
  std::deque<std::vector<float>> melSpectrumHistory;
  std::deque<std::vector<float>> waveformHistory;
  std::vector<float> peakSpectrum;
  std::vector<float> peakMelSpectrum;
  std::vector<float> peakWaveform;
  int movingAverageWindowSize = 3;

  AudioAnalysisParameter rmsAnalysisParam;  
  AudioAnalysisParameter highsAnalysisParam;
  AudioAnalysisParameter midsAnalysisParam;
  AudioAnalysisParameter lowsAnalysisParam;
  
  u_int64_t bpmStartTime;
  
  std::vector<std::shared_ptr<Parameter>> parameters;
  std::vector<AudioAnalysisParameter *> analysisParameters;
  
  void zero() {
    rms->value = 0.0f;
    beatPulse->value = 0.0f;
  }

  // Thread-safe accessors for audio data
  std::vector<float> getSafeSpectrum() const {
    return smoothSpectrum;
  }

  std::vector<float> getSafeMelSpectrum() const {
    return smoothMelSpectrum;
  }

  std::vector<float> getSafeWaveform() const {
    return smoothWaveform;
  }

  AudioAnalysis()
      : rms(std::make_shared<Parameter>("Loudness", 0.0, 0.0, 1.0)),
        highs(std::make_shared<Parameter>("Highs", 0.0, 0.0, 1.0)),
        mids(std::make_shared<Parameter>("Mids", 0.0, 0.0, 1.0)),
        lows(std::make_shared<Parameter>("Lows", 0.0, 0.0, 1.0)),
        beatPulse(std::make_shared<Parameter>("BPM", 0.0, 0.0, 1.0)),
        bpm(std::make_shared<Parameter>("bpm", 120.0, 0.0, 300.0)),
        bpmModeParam(std::make_shared<Parameter>("", static_cast<float>(BpmMode_Auto), 0.0, 2.0)),
        bpmLocked(std::make_shared<Parameter>("BPM Locked", 0.0, ParameterType_Bool)),
        bpmNudge(std::make_shared<Parameter>("BPM Nudge", 0.0, -1.0, 1.0)),
        frequencyRelease(std::make_shared<Parameter>("Release", 0.7, 0.01, 1.0)),
        frequencyScale(std::make_shared<Parameter>("Scale", 1.0, 0.01, 2.0)),
        rmsOscillator(std::make_shared<ValueOscillator>(rms)),
        beatPulseOscillator(std::make_shared<PulseOscillator>(beatPulse)),
        highsOscillator(std::make_shared<ValueOscillator>(highs)),
        midsOscillator(std::make_shared<ValueOscillator>(mids)),
        lowsOscillator(std::make_shared<ValueOscillator>(lows)),
        rmsAnalysisParam(AudioAnalysisParameter(rms)),
        highsAnalysisParam(AudioAnalysisParameter(highs)),
        midsAnalysisParam(AudioAnalysisParameter(mids)),
        lowsAnalysisParam(AudioAnalysisParameter(lows)),
        smoothMelSpectrum({0.,0.,0.,0.,0.,0.,0.,0.,0.,0.,0.,0.,0.}),
  			// TODO: Readd/fix Pulser
        parameters({beatPulse, rms, highs, mids, lows}),
        analysisParameters({&rmsAnalysisParam, &lowsAnalysisParam, &midsAnalysisParam, &highsAnalysisParam}) {
    // Initialize bpmStartTime to current time
    auto currentTime = std::chrono::steady_clock::now();
    bpmStartTime = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime.time_since_epoch()).count();

    // Sync bpmMode with bpmModeParam
    bpmModeParam->intValue = static_cast<int>(bpmMode);
  };

  std::vector<float> applySmoothing(const std::vector<float>& input, std::vector<float>& previous,
                                     std::deque<std::vector<float>>& history, std::vector<float>& peak) {
    std::vector<float> result;

    switch (smoothingMode) {
      case SmoothingMode_Exponential:
        // Existing exponential smoothing
        result = Vectors::release(input, previous, frequencyRelease->value);
        break;

      case SmoothingMode_MovingAverage: {
        // Add current input to history
        history.push_back(input);
        if (history.size() > movingAverageWindowSize) {
          history.pop_front();
        }

        // Calculate moving average
        if (history.empty()) {
          result = input;
        } else {
          result = std::vector<float>(input.size(), 0.0f);
          for (const auto& frame : history) {
            for (size_t i = 0; i < result.size() && i < frame.size(); i++) {
              result[i] += frame[i];
            }
          }
          for (size_t i = 0; i < result.size(); i++) {
            result[i] /= history.size();
          }
        }
        break;
      }

      case SmoothingMode_None:
        // No smoothing, just return input
        result = input;
        break;

      case SmoothingMode_PeakHold: {
        // Initialize peak if empty
        if (peak.empty()) {
          peak = input;
        }

        result = std::vector<float>(input.size(), 0.0f);
        for (size_t i = 0; i < input.size() && i < peak.size(); i++) {
          // Update peak if current value is higher
          if (input[i] > peak[i]) {
            peak[i] = input[i];
          } else {
            // Decay peak
            peak[i] *= frequencyRelease->value;
          }
          result[i] = peak[i];
        }
        break;
      }
    }

    return result;
  }

  void analyzeFrame(Gist<float> *gist) {
    magnitudeSpectrum = gist->getMagnitudeSpectrum();
    melFrequencySpectrum =
        Vectors::normalize(Vectors::sqrt(gist->getMelFrequencySpectrum()));

    smoothSpectrum = applySmoothing(magnitudeSpectrum, smoothSpectrum, spectrumHistory, peakSpectrum);
    smoothSpectrum = Vectors::scalarMultiply(smoothSpectrum, frequencyScale->value);
    // Clamp spectrum to [0, 1] to prevent out-of-bounds spikes
    for (size_t i = 0; i < smoothSpectrum.size(); i++) {
      smoothSpectrum[i] = std::max(0.0f, std::min(1.0f, smoothSpectrum[i]));
    }

    smoothMelSpectrum = applySmoothing(melFrequencySpectrum, smoothMelSpectrum, melSpectrumHistory, peakMelSpectrum);
    smoothMelSpectrum = Vectors::scalarMultiply(smoothMelSpectrum, frequencyScale->value);
    // Clamp mel spectrum to [0, 1] to prevent out-of-bounds spikes
    for (size_t i = 0; i < smoothMelSpectrum.size(); i++) {
      smoothMelSpectrum[i] = std::max(0.0f, std::min(1.0f, smoothMelSpectrum[i]));
    }

    // Apply smoothing to waveform (bipolar signal - handle positive and negative)
    if (waveform.size() == smoothWaveform.size()) {
      smoothWaveform = applySmoothing(waveform, smoothWaveform, waveformHistory, peakWaveform);
      smoothWaveform = Vectors::scalarMultiply(smoothWaveform, frequencyScale->value);
      // Clamp waveform to [-1, 1] to prevent out-of-bounds spikes
      for (size_t i = 0; i < smoothWaveform.size(); i++) {
        smoothWaveform[i] = std::max(-1.0f, std::min(1.0f, smoothWaveform[i]));
      }
    } else {
      // Initialize smoothWaveform if sizes don't match
      smoothWaveform = waveform;
    }
    
    rmsAnalysisParam.tick(gist->rootMeanSquare());
    std::vector<float> buckets = splitAndAverage(smoothMelSpectrum, 3);
    lows->setValue(buckets[0]);
    mids->setValue(buckets[1]);
    highs->setValue(buckets[2]);
  }

  float bpmPct() {
    auto currentTime = std::chrono::steady_clock::now();
    auto currentTimeMs = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime.time_since_epoch()).count();

    auto elapsedTime = (currentTimeMs - bpmStartTime) / 1000.0;
    float beatDuration = 60.0f / bpm->value;

    // Calculate the percentage and how close we are to the next beat
    float pct = fmod(elapsedTime, beatDuration) / beatDuration;

    // Calculate how many milliseconds until the next beat
    float msUntilNextBeat = (1.0f - pct) * beatDuration * 1000;

    // If we're very close to the next beat (within 1 frame @ 60fps), snap to 1.0
    if (msUntilNextBeat < 17) {
      return 1.0f;
    }

    // If we're very close to the start of a beat, snap to 0.0
    if (pct < 0.017) { // 17ms / beatDuration
      return 0.0f;
    }

    return pct;
  }

  // Adjust bpmStartTime to preserve the current beat phase when BPM value changes
  void adjustBpmStartTimeForPhase(float newBpm) {
    // Calculate current phase before BPM changes
    float currentPct = bpmPct();

    // Update BPM value
    bpm->setValue(newBpm);

    // Calculate new beat duration
    float newBeatDuration = 60.0f / newBpm;

    // Adjust bpmStartTime so that bpmPct() returns the same phase
    auto currentTime = std::chrono::steady_clock::now();
    auto currentTimeMs = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime.time_since_epoch()).count();

    // New start time = current time - (phase * beat duration)
    bpmStartTime = currentTimeMs - static_cast<u_int64_t>(currentPct * newBeatDuration * 1000);
  }

  std::vector<float> splitAndAverage(const std::vector<float>& frequencies, int count) {
      std::vector<float> averages;
      if (count <= 0 || frequencies.empty()) {
          // Return an empty vector if count is non-positive or frequencies is empty
          return averages;
      }

      int totalSize = frequencies.size();
      int baseSize = totalSize / count;
      int remainder = totalSize % count;

      int start = 0;
      for (int i = 0; i < count; ++i) {
          // Calculate the size of the current split, adding an extra element if remainder > 0
          int currentSize = baseSize + (i < remainder ? 1 : 0);
          if (start + currentSize > totalSize) {
              // Avoid going out of bounds
              break;
          }

          // Calculate the average for the current split
          float sum = std::accumulate(frequencies.begin() + start, frequencies.begin() + start + currentSize, 0.0f);
          float average = sum / currentSize;
          averages.push_back(average);

          // Update the start index for the next split
          start += currentSize;
      }

      return averages;
  }


float gammaAtBeat(float pct) {
    // Use a gamma distribution function
    // Constants A, alpha, and beta can be adjusted to change the shape of the pulse
    const float A = 1.0f;    // Amplitude
    const float alpha = 2.0f; // Shape parameter
    const float beta = 1.0f;  // Rate parameter
    
    // Ensure pct is in the range [0, 1]
    if (pct < 0.0f) pct = 0.0f;
    if (pct > 1.0f) pct = 1.0f;

    // Calculate the gamma distribution value at pct
    // Since pct is between 0 and 1, we need to scale it appropriately
    float t = pct; // In this case, we can use pct directly as t
    
    // Gamma distribution function y(t) = A * t^(alpha - 1) * exp(-beta * t)
    float value = A * std::pow(t, alpha - 1) * std::exp(-beta * t);

    return value;
}

  
  double gammapdf(double value, double alpha, double beta) {
    return 0.0;
  }

  void updateBeat(float pct) {
    if (bpmEnabled) {
      if (pct < 0 || pct > 1) {
        return;
      }

      // Apply nudge value (-1 to 1, representing a full beat shift)
      pct += bpmNudge->value;

      // Wrap around if nudge pushes us outside [0, 1]
      while (pct < 0.0f) pct += 1.0f;
      while (pct >= 1.0f) pct -= 1.0f;

      // Constants for exponential decay
      const float A = 1.0f;    // Amplitude
      const float B = -5.0f;   // Controls the steepness
      const float C = 0.0f;    // Offset

      // If we're at exactly 1.0 (snapped), ensure we hit peak amplitude
      if (pct >= 1.0f) {
        beatPulse->value = A;
        return;
      }

      float y = A * exp(B * pct) + C;
      beatPulse->value = y;
    }
  }
};

#endif /* AudioSettings_h */
