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

  std::shared_ptr<Parameter> param;
  accumulator_set<double, stats<tag::rolling_mean > > valueAcc;
  accumulator_set<double, stats<tag::max, tag::min>> minMaxAcc;

  AudioAnalysisParameter(std::shared_ptr<Parameter> param) :
  valueAcc(tag::rolling_window::window_size = 5),
  minMaxAcc(tag::rolling_window::window_size = 400),
  param(param) {}

  void tick(float val) {
    value = val;
    valueAcc(val);
    minMaxAcc(val);
    windowMin = boost::accumulators::min(minMaxAcc);
    windowMax = boost::accumulators::max(minMaxAcc);
    
    rollingMean = rolling_mean(valueAcc);
    rollingMeanRelation = relationToRange(rollingMean);
    
    param->value = rollingMeanRelation;
  }

  float relationToRange(float v) {
    float range = windowMax - windowMin;
    return v / range;
  }
};

struct AudioAnalysis {
  std::string name;
  bool toggled = false;
  std::shared_ptr<Parameter> rms;
  std::shared_ptr<Parameter> csd;
  std::shared_ptr<Parameter> pitch;
  std::shared_ptr<Parameter> energy;
  std::shared_ptr<Parameter> zcr;
  std::shared_ptr<Parameter> spectralDiff;
  std::shared_ptr<Parameter> beat;

  std::shared_ptr<ValueOscillator> rmsOscillator;
  std::shared_ptr<ValueOscillator> csdOscillator;
  std::shared_ptr<ValueOscillator> pitchOscillator;
  std::shared_ptr<ValueOscillator> energyOscillator;
  std::shared_ptr<ValueOscillator> zcrOscillator;
  std::shared_ptr<ValueOscillator> spectralDiffOscillator;
  std::shared_ptr<PulseOscillator> beatOscillator;

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

  AudioAnalysis()
      : rms(std::make_shared<Parameter>("rms", name, 0.0, 0.0, 1.0)),
        csd(std::make_shared<Parameter>("csd", name, 0.0, 0.0, 1.0)),
        pitch(std::make_shared<Parameter>("pitch", name, 0.0, 0.0, 1.0)),
        energy(std::make_shared<Parameter>("energy", name, 0.0, 0.0, 1.0)),
        zcr(std::make_shared<Parameter>("zcr", name, 0.0, 0.0, 1.0)),
        spectralDiff(
            std::make_shared<Parameter>("spectralDiff", name, 0.0, 0.0, 1.0)),
        beat(std::make_shared<Parameter>("beat", name, 0.0, 0.0, 1.0)),
        rmsOscillator(std::make_shared<ValueOscillator>(rms)),
        pitchOscillator(std::make_shared<ValueOscillator>(pitch)),
        csdOscillator(std::make_shared<ValueOscillator>(csd)),
        zcrOscillator(std::make_shared<ValueOscillator>(zcr)),
        energyOscillator(std::make_shared<ValueOscillator>(energy)),
        spectralDiffOscillator(std::make_shared<ValueOscillator>(spectralDiff)),
        beatOscillator(std::make_shared<PulseOscillator>(beat)),
        parameters({rms, csd, pitch, energy, zcr, beat}),
        rmsAnalysisParam(AudioAnalysisParameter(rms)),
        csdAnalysisParam(AudioAnalysisParameter(csd)),
        pitchAnalysisParam(AudioAnalysisParameter(pitch)),
        energyAnalysisParam(AudioAnalysisParameter(energy)),
        zcrAnalysisParam(AudioAnalysisParameter(zcr)),
        spectralDiffAnalysisParam(AudioAnalysisParameter(spectralDiff)),
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
};

#endif /* AudioSettings_h */
