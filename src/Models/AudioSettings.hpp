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
#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/framework/accumulator_set.hpp>
#include <boost/accumulators/framework/extractor.hpp>
#include <boost/accumulators/statistics/rolling_count.hpp>
#include <boost/accumulators/statistics/rolling_mean.hpp>
#include <boost/accumulators/statistics/max.hpp>
#include <boost/accumulators/statistics/min.hpp>
#include <boost/accumulators/statistics/rolling_sum.hpp>
#include <boost/accumulators/statistics/rolling_window.hpp>
#include <boost/accumulators/statistics/stats.hpp>

using namespace boost::accumulators;

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
  
  accumulator_set<double, stats<tag::rolling_mean > > valueAcc;
  accumulator_set<double, stats<tag::max, tag::min>> minMaxAcc;

  AudioAnalysisParameter(std::shared_ptr<Parameter> param) :
  valueAcc(tag::rolling_window::window_size = 2),
  minMaxAcc(tag::rolling_window::window_size = 800),
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
    valueAcc(val);
    minMaxAcc(val);
    windowMin = boost::accumulators::min(minMaxAcc);
    windowMax = boost::accumulators::max(minMaxAcc);
    
    rollingMean = rolling_mean(valueAcc);
    rollingMeanRelation = relationToRange(rollingMean);
    
    param->value = rollingMeanRelation;
    pulse->value = pulser.next(rollingMeanRelation);
  }

  float relationToRange(float v) {
    float range = windowMax - windowMin;
    return v / range;
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
  
  // Enable pulsers
  std::shared_ptr<Parameter> enableRmsPulse;

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
        frequencyRelease(std::make_shared<Parameter>("Frequency Release", 0.95, 0.5, 1.0)),
        enableRmsPulse(std::make_shared<Parameter>("Pulser", 0.0, 0.0, 1.0)),
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
        parameters({rms, beatPulse, highs, mids, lows, enableRmsPulse}),
        analysisParameters({&rmsAnalysisParam, &lowsAnalysisParam, &midsAnalysisParam, &highsAnalysisParam
  }){};

  void analyzeFrame(Gist<float> *gist) {
    magnitudeSpectrum =
        Vectors::normalize(Vectors::sqrt(gist->getMagnitudeSpectrum()));
    melFrequencySpectrum =
        Vectors::normalize(Vectors::sqrt(gist->getMelFrequencySpectrum()));
    smoothSpectrum = Vectors::release(magnitudeSpectrum, smoothSpectrum, frequencyRelease->value);
    smoothMelSpectrum =
        Vectors::release(melFrequencySpectrum, smoothMelSpectrum, frequencyRelease->value);
    
    std::vector<float> highsMidsLows = splitAndAverage(smoothMelSpectrum, 3);
    if (highsMidsLows.size() == 3) {
      highsAnalysisParam.tick(highsMidsLows[2]);
      midsAnalysisParam.tick(highsMidsLows[1]);
      lowsAnalysisParam.tick(highsMidsLows[0]);
    }
    
    rmsAnalysisParam.tick(gist->rootMeanSquare());
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
  
  void updateBeat(float pct) {
    if (bpmEnabled) {
      if (pct < 0 || pct > 1) {
          // Handle out of range input
          return 0.0f;
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
  
  float sineAtBPM(float bpm) {
      // Convert bpm to frequency in Hz
      float frequency = bpm / 60.0f;

      // Get the current time in seconds since the app started
      float seconds = ofGetElapsedTimef();

      // Calculate the phase of the sine wave, using the frequency.
      // The calculation is: 2 * PI * frequency * time
      float phase = 2.0f * PI * frequency * seconds;

      // Return the sine of the phase, which is the wave's value at the current time
      return sin(phase);
  }
};

#endif /* AudioSettings_h */
