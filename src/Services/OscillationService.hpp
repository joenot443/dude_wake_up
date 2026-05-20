//
//  OscillationService.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 2022-07-05.
//

#ifndef OscillationService_hpp
#define OscillationService_hpp

#include <stdio.h>
#include <vector>
#include <deque>
#include "Oscillator.hpp"
#include "ValueOscillator.hpp"
#include "PulseOscillator.hpp"
#include "ConfigurableService.hpp"

class OscillationService: public ConfigurableService {
private:
  // map of form:
  /*
   "HSB_hue": {
       "amp": {
           "midiDescriptor": "",
           "name": "amp",
           "paramId": "amp-ec9032e6",
           "value": 1.0
       },
   }
   */
  std::map<std::string, std::map<std::string, float>> oscillatorSettings;
  
public:
  static OscillationService* service;
  std::shared_ptr<Oscillator> selectedOscillator;

  OscillationService() {};
  static OscillationService* getService() {
    if (!service) {
      service = new OscillationService;
    }
    return service;
    
  }
  std::map<std::string, std::shared_ptr<Oscillator>> oscillators;
  std::set<std::shared_ptr<Oscillator>> activeOscillators();

  void selectOscillator(std::shared_ptr<Oscillator>, std::shared_ptr<Parameter>);
  void addOscillator(std::shared_ptr<Oscillator> o);
  void loadOscillatorSettings(std::shared_ptr<Oscillator> o);
  std::shared_ptr<Oscillator> oscillatorForParameter(std::shared_ptr<Parameter> param);
  void tickOscillators();

  // Sets enabled state on an oscillator and keeps the hot list in sync.
  // All callers that toggle Oscillator::enabled MUST go through this helper.
  void setEnabled(std::shared_ptr<Oscillator> osc, bool enabled);

  void clear();

  json config() override;
  void loadConfig(json j) override;

  void removeOscillatorsFor(std::vector<std::shared_ptr<Parameter>> parameters);

private:
  // Hot list of currently-enabled oscillators. tickOscillators iterates only
  // this — avoids walking the full oscillators map every frame.
  std::vector<std::shared_ptr<Oscillator>> enabledOscillators_;
  void rebuildEnabledCache();
};

#endif /* OscillationService_hpp */
