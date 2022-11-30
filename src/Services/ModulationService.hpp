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
#include "Parameter.hpp"
#include "AudioSettings.hpp"

struct ModulationSetting {
  std::shared_ptr<Parameter> videoParam;
  std::shared_ptr<Parameter> audioParam;
  ModulationSetting(std::shared_ptr<Parameter> p = NULL, std::shared_ptr<Parameter> a = NULL) : videoParam(p), audioParam(a) {};
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
  
  std::shared_ptr<Parameter> audioParameterDriving(std::shared_ptr<Parameter> videoParam);
  bool videoParameterIsBeingDriven(std::shared_ptr<Parameter> videoParam);
  void addAudioAnalysis(AudioAnalysis *analysis);
  void addMapping(std::shared_ptr<Parameter> audioParam, std::shared_ptr<Parameter> videoParam);
  void removeMapping(std::shared_ptr<Parameter> videoParam);
  void tickMappings();
};



#endif /* ModulationService_hpp */
