//
//  AudioSourceService.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 12/21/22.
//

#ifndef AudioSourceService_hpp
#define AudioSourceService_hpp

#include "AudioSource.hpp"
#include "MSABPMTapper.h"
#include "ConfigurableService.hpp"
#include "Parameter.hpp"
#include "Link.hpp"
#include <stdio.h>

class AudioSourceService : public ConfigurableService {
private:
  std::map<std::string, std::shared_ptr<AudioSource>> audioSourceMap;
  std::map<std::string, std::string> audioSourceIdToParamId;

public:
  static AudioSourceService *service;
  std::shared_ptr<AudioSource> defaultAudioSource;
  std::shared_ptr<AudioSource> selectedAudioSource;
  ableton::Link link;

  AudioSourceService() : link(120.0) {};
  
  static AudioSourceService *getService() {
    if (!service) {
      service = new AudioSourceService;
      service->setup();
    }
    return service;
  }
  void setup();
  void selectAudioSource(std::shared_ptr<AudioSource> source);
  
  void update();

  msa::BPMTapper tapper;
  
  std::vector<std::shared_ptr<AudioSource>> audioSources();
  std::shared_ptr<AudioSource> audioSourceForId(std::string id);
  std::shared_ptr<AudioSource> audioSourceForParamId(std::string paramId);

  void removeParamMapping(std::string paramId);

  json config() override;
  void loadConfig(json j) override;
};

#endif /* AudioSourceService_hpp */
