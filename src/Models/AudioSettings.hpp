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
#include "Gist.h"
#include "Math.hpp"
#include "Oscillator.hpp"
#include "Parameter.hpp"
#include "Pulser.hpp"
#include "PulseOscillator.hpp"
#include "ValueOscillator.hpp"
#include "Vectors.hpp"
#include "RollingMean.hpp" // Include the new RollingMean class

struct AudioAnalysisParameter {
  float windowMin = 0.0;
  float windowMax = 0.0;

  float value = 0.0;
  float rollingMean = 0.0;
  
  // Vary from 0.0 to 1.0: rollingMean / windowMax:
  float rollingMeanRelation = 0.0;
  
  Pulser pulser;

  std::shared_ptr<Parameter> pulseLength;
  std::shared_ptr<Parameter> pulseThreshold;
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
  pulse(std::make_shared<Parameter>("autoPulse", 0.0, 0.0, 1.0)),
  pulseOscillator(std::make_shared<ValueOscillator>(pulse)),
  thresholdOscillator(std::make_shared<ValueOscillator>(pulseThreshold)),
  pulser(pulseThreshold, pulseLength),
  param(param) {
    pulser = Pulser(pulseThreshold, pulseLength);
  }

  void tick(float val) {
    value = val;
    valueAcc.add(val);  // Add value to RollingMean accumulator

    // Update the min/max deque
    minMaxValues.push_back(val);
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
  bool bpmEnabled = false;
  std::shared_ptr<Parameter> rms;
  std::shared_ptr<Parameter> highs;
  std::shared_ptr<Parameter> lows;
  std::shared_ptr<Parameter> mids;
  std::shared_ptr<Parameter> beatPulse;
  std::shared_ptr<Parameter> beatOscillatorSelection;
  std::shared_ptr<Parameter> bpm;
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

  AudioAnalysisParameter rmsAnalysisParam;  
  AudioAnalysisParameter highsAnalysisParam;
  AudioAnalysisParameter midsAnalysisParam;
  AudioAnalysisParameter lowsAnalysisParam;

  std::vector<std::shared_ptr<Parameter>> parameters;
  std::vector<AudioAnalysisParameter *> analysisParameters;
  
  void zero() {
    rms->value = 0.0f;
    beatPulse->value = 0.0f;
  }

  AudioAnalysis()
      : rms(std::make_shared<Parameter>("Loudness", 0.0, 0.0, 1.0)),
        highs(std::make_shared<Parameter>("Highs", 0.0, 0.0, 1.0)),
        mids(std::make_shared<Parameter>("Mids", 0.0, 0.0, 1.0)),
        lows(std::make_shared<Parameter>("Lows", 0.0, 0.0, 1.0)),
        beatPulse(std::make_shared<Parameter>("BPM", 0.0, 0.0, 1.0)),
        bpm(std::make_shared<Parameter>("bpm", 60.0, 0.0, 300.0)),
        frequencyRelease(std::make_shared<Parameter>("Frequency Release", 0.95, 0.0, 1.0)),
        frequencyScale(std::make_shared<Parameter>("Frequency Scale", 1.0, 0.0, 5.0)),
        rmsOscillator(std::make_shared<ValueOscillator>(rms)),
        beatPulseOscillator(std::make_shared<PulseOscillator>(beatPulse)),
        highsOscillator(std::make_shared<ValueOscillator>(highs)),
        midsOscillator(std::make_shared<ValueOscillator>(mids)),
        lowsOscillator(std::make_shared<ValueOscillator>(lows)),
        rmsAnalysisParam(AudioAnalysisParameter(rms)),
        highsAnalysisParam(AudioAnalysisParameter(highs)),
        midsAnalysisParam(AudioAnalysisParameter(mids)),
        lowsAnalysisParam(AudioAnalysisParameter(lows)),
  			// TODO: Readd/fix Pulser
        parameters({beatPulse, rms, highs, mids, lows}),
        analysisParameters({&rmsAnalysisParam, &lowsAnalysisParam, &midsAnalysisParam, &highsAnalysisParam}){};

  void analyzeFrame(Gist<float> *gist) {
    magnitudeSpectrum = gist->getMagnitudeSpectrum();
//        Vectors::normalize(Vectors::sqrt(gist->getMagnitudeSpectrum()));
    melFrequencySpectrum =
        Vectors::normalize(Vectors::sqrt(gist->getMelFrequencySpectrum()));
    smoothSpectrum = Vectors::release(magnitudeSpectrum, smoothSpectrum, frequencyRelease->value);
    smoothSpectrum = Vectors::scalarMultiply(smoothSpectrum, frequencyScale->value);
    smoothMelSpectrum =
        Vectors::release(melFrequencySpectrum, smoothMelSpectrum, frequencyRelease->value);
    smoothMelSpectrum = Vectors::scalarMultiply(smoothMelSpectrum, frequencyScale->value);
    
    rmsAnalysisParam.tick(gist->rootMeanSquare());
    std::vector<float> buckets = splitAndAverage(magnitudeSpectrum, 3);
    lows->setValue(buckets[0]);
    mids->setValue(buckets[1]);
    highs->setValue(buckets[2]);
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
          // Handle out of range input
          return;
      }
      if (pct > 0.95) {
        pct = 1.0f;
      }
      
      // Constants A, B, and C can be adjusted to change the shape of the pulse
      const float A = 1.0f; // Amplitude
      const float B = -5.0f; // Controls the steepness
      const float C = 0.0f; // Offset

      float y = A * exp(B * pct) + C;
      beatPulse->value = y;
    }
  }
};

#endif /* AudioSettings_h */
