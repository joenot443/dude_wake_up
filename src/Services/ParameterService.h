//
//  ParameterService.h
//  dude_wake_up
//
//  Created by Joe Crozier on 2022-07-24.
//

#ifndef ParameterService_h
#define ParameterService_h

#include <stdio.h>
#include "json.hpp"
#include "ofxMidi.h"
#include "ofMain.h"
#include "MidiPairing.hpp"
#include "Parameter.h"

using json = nlohmann::json;

class ParameterService {
  std::map<std::string, Parameter *> parameterMap;
    
public:
  Parameter * parameterForId(std::string paramId);
  static ParameterService* service;
  
  void registerParameter(Parameter *);
  void loadParameters(json *);
  
  ParameterService() {};
  static ParameterService* getService() {
    if (!service) {
      service = new ParameterService;
    }
    return service;
    
  }
};

#endif /* ParameterService_h */
