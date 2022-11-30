//
//  ParameterService.hpp
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
#include "Parameter.hpp"

using json = nlohmann::json;

class ParameterService {
  
    
public:
  std::shared_ptr<Parameter> parameterForId(std::string paramId);
  static ParameterService* service;
  
  void registerParameter(std::shared_ptr<Parameter> parameter);
  std::map<std::string, std::shared_ptr<Parameter>> parameterMap;
  
  std::shared_ptr<Parameter> selectedParameter;
  
  ParameterService() {};
  static ParameterService* getService() {
    if (!service) {
      service = new ParameterService;
    }
    return service;
    
  }
};

#endif /* ParameterService_h */
