//
//  ParameterService.h
//  dude_wake_up
//
//  Created by Joe Crozier on 2022-07-24.
//

#ifndef ParameterService_h
#define ParameterService_h

#include <stdio.h>
#include "ofxMidi.h"
#include "ofMain.h"
#include "MidiPairing.hpp"
#include "Parameter.h"

class ParameterService {
  std::map<std::string, Parameter *> parameterMap;
  
  void setup();
  
public:
  Parameter * parameterForId(std::string paramId);
  static ParameterService* service;
  
  void registerParameter(Parameter *);
  
  ParameterService() {};
  static ParameterService* getService() {
    if (!service) {
      service = new ParameterService;
      service->setup();
    }
    return service;
    
  }
};

#endif /* ParameterService_h */
