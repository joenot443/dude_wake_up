//
//  AudioSettings.h
//  dude_wake_up
//
//  Created by Joe Crozier on 2022-05-30.
//

#ifndef AudioSettings_h
#define AudioSettings_h

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
  std::shared_ptr<Parameter> csd;
  std::shared_ptr<Parameter> pitch;
  std::shared_ptr<Parameter> energy;
  std::shared_ptr<Parameter> zcr;
  std::shared_ptr<Parameter> spectralDiff;
  std::shared_ptr<Parameter> beatPulse;
  std::shared_ptr<Parameter> beatOscillatorSelection;
  std::shared_ptr<Parameter> bpm;

  std::shared_ptr<ValueOscillator> rmsOscillator;
  std::shared_ptr<ValueOscillator> csdOscillator;
  std::shared_ptr<ValueOscillator> pitchOscillator;
  std::shared_ptr<ValueOscillator> energyOscillator;
  std::shared_ptr<ValueOscillator> zcrOscillator;
  std::shared_ptr<ValueOscillator> spectralDiffOscillator;
  std::shared_ptr<PulseOscillator> beatPulseOscillator;

  std::vector<float> melFrequencySpectrum;
  std::vector<float> magnitudeSpectrum;
  std::vector<float> smoothSpectrum;
  std::vector<float> smoothMelSpectrum;

  AudioAnalysisParameter rmsAnalysisParam;
  AudioAnalysisParameter csdAnalysisParam;
  AudioAnalysisParameter pitchAnalysisParam;
  AudioAnalysisParameter energyAnalysisParam;
  AudioAnalysisParameter zcrAnalysisParam;
  AudioAnalysisParameter spectralDiffAnalysisParam;

  std::vector<std::shared_ptr<Parameter>> parameters;
  std::vector<AudioAnalysisParameter *> analysisParameters;
  
  void zero() {
    rms->value = 0.0f;
    csd->value = 0.0f;
    energy->value = 0.0f;
    pitch->value = 0.0f;
    zcr->value = 0.0f;
    spectralDiff->value = 0.0f;
    beatPulse->value = 0.0f;
  }

  AudioAnalysis()
      : rms(std::make_shared<Parameter>("rms", 0.0, 0.0, 1.0)),
        csd(std::make_shared<Parameter>("csd", 0.0, 0.0, 1.0)),
        pitch(std::make_shared<Parameter>("pitch", 0.0, 0.0, 1.0)),
        energy(std::make_shared<Parameter>("energy", 0.0, 0.0, 1.0)),
        zcr(std::make_shared<Parameter>("zcr", 0.0, 0.0, 1.0)),
        spectralDiff(
            std::make_shared<Parameter>("spectralDiff", 0.0, 0.0, 1.0)),
        beatPulse(std::make_shared<Parameter>("beatPulse", 0.0, 0.0, 1.0)),
        bpm(std::make_shared<Parameter>("bpm", 60.0, 0.0, 300.0)),
        rmsOscillator(std::make_shared<ValueOscillator>(rms)),
        pitchOscillator(std::make_shared<ValueOscillator>(pitch)),
        csdOscillator(std::make_shared<ValueOscillator>(csd)),
        zcrOscillator(std::make_shared<ValueOscillator>(zcr)),
        energyOscillator(std::make_shared<ValueOscillator>(energy)),
        spectralDiffOscillator(std::make_shared<ValueOscillator>(spectralDiff)),
        beatPulseOscillator(std::make_shared<PulseOscillator>(beatPulse)),
        rmsAnalysisParam(AudioAnalysisParameter(rms)),
        csdAnalysisParam(AudioAnalysisParameter(csd)),
        pitchAnalysisParam(AudioAnalysisParameter(pitch)),
        energyAnalysisParam(AudioAnalysisParameter(energy)),
        zcrAnalysisParam(AudioAnalysisParameter(zcr)),
        spectralDiffAnalysisParam(AudioAnalysisParameter(spectralDiff)),
        parameters({rms, csd, pitch, energy, zcr, beatPulse, rmsAnalysisParam.pulse}),
        analysisParameters({&rmsAnalysisParam, &csdAnalysisParam,
                            &pitchAnalysisParam, &energyAnalysisParam,
                            &zcrAnalysisParam, &spectralDiffAnalysisParam}){};

  void analyzeFrame(Gist<float> *gist) {
    magnitudeSpectrum =
        Vectors::normalize(Vectors::sqrt(gist->getMagnitudeSpectrum()));
    melFrequencySpectrum =
        Vectors::normalize(Vectors::sqrt(gist->getMelFrequencySpectrum()));

    smoothSpectrum = Vectors::release(magnitudeSpectrum, smoothSpectrum);
    smoothMelSpectrum =
        Vectors::release(melFrequencySpectrum, smoothMelSpectrum);

    rmsAnalysisParam.tick(gist->rootMeanSquare());
    pitchAnalysisParam.tick(gist->pitch());
    csdAnalysisParam.tick(gist->complexSpectralDifference());
    energyAnalysisParam.tick(gist->energyDifference());
    zcrAnalysisParam.tick(gist->zeroCrossingRate());
    spectralDiffAnalysisParam.tick(gist->spectralDifference());
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
};

#endif /* AudioSettings_h */
