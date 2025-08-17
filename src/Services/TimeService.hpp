#ifndef TimeService_hpp
#define TimeService_hpp

#include <stdio.h>
#include <vector>
#include <map>
#include <tuple>
#include "ofMain.h"
#include "Parameter.hpp"
#include "WaveformOscillator.hpp"

class TimeService {
public:

  std::shared_ptr<Parameter> timeParam;
  std::shared_ptr<Parameter> timeSpeedParam;
  std::shared_ptr<Parameter> timeOffsetParam;

  std::shared_ptr<WaveformOscillator> timeOscillator;
  std::shared_ptr<WaveformOscillator> timeSpeedOscillator;
  std::shared_ptr<WaveformOscillator> timeOffsetOscillator;

  bool paused;

  static TimeService* service;
  TimeService() : timeParam(std::make_shared<Parameter>("Time", 0.0f, 0.0f, 1000000000.0f)),
  timeSpeedParam(std::make_shared<Parameter>("Time Speed", 1.0f, 0.0f, 10.0f)),
  timeOffsetParam(std::make_shared<Parameter>("Time Offset", 0.0f, 0.0f, 10.0f)),
  timeOscillator(std::make_shared<WaveformOscillator>(timeParam)),
  timeSpeedOscillator(std::make_shared<WaveformOscillator>(timeSpeedParam)),
  timeOffsetOscillator(std::make_shared<WaveformOscillator>(timeOffsetParam)) {};
  static TimeService* getService() {
    if (!service) {
      service = new TimeService;
    }
    return service;
  }

  void setup();

  void tick() {
    float newTime = ofGetElapsedTimef() * timeSpeedParam->value + timeOffsetParam->value;
    timeParam->value = newTime;
  }
};

#endif /* TimeService_hpp */
