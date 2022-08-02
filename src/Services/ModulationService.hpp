//
//  ModulationService.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 2022-07-04.
//

#ifndef ModulationService_hpp
#define ModulationService_hpp

#include <stdio.h>
#include <vector>
#include <map>
#include <tuple>
#include "Parameter.h"
#include "AudioSettings.h"

struct ModulationSetting {
  Parameter * videoParam;
  Parameter * audioParam;
  ModulationSetting(Parameter *v = NULL, Parameter *a = NULL) : videoParam(v), audioParam(a) {};
};

class ModulationService {
private:
  
  std::map<std::string, ModulationSetting> mappings;
  
public:
  static ModulationService* service;
  ModulationService() {};
  static ModulationService* getService() {
    if (!service) {
      service = new ModulationService;
    }
    return service;
    
  }
  std::vector<AudioAnalysis*> audioAnalysis;
  
  Parameter* audioParameterDriving(Parameter *videoParam);
  bool videoParameterIsBeingDriven(Parameter *videoParam);
  void addAudioAnalysis(AudioAnalysis *analysis);
  void addMapping(Parameter *audioParam, Parameter *videoParam);
  void removeMapping(Parameter *videoParam);
  void tickMappings();
};



#endif /* ModulationService_hpp */
