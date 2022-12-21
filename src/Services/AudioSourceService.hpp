//
//  AudioSourceService.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 12/21/22.
//

#ifndef AudioSourceService_hpp
#define AudioSourceService_hpp

#include <stdio.h>
#include "ConfigurableService.hpp"
#include "Parameter.hpp"
#include "AudioSource.hpp"

class AudioSourceService: public ConfigurableService {
  private:
  std::map<std::string, std::shared_ptr<AudioSource>> audioSourceMap;
  std::map<std::string, std::string> audioSourceIdToParamId;

  static AudioSourceService* service;
  AudioSourceService() {};
  static AudioSourceService* getService() {
    if (!service) {
      service = new AudioSourceService;
      service->setup();
    }
    return service;
  }

  public:
  void setup();
  void updateAudioSources();
  std::vector<std::shared_ptr<AudioSource>> audioSources();
  std::shared_ptr<AudioSource> audioSourceForId(std::string id);
  std::shared_ptr<AudioSource> audioSourceForParamId(std::string paramId);
  void removeParamMapping(std::string paramId);

  json config() override;
  void loadConfig(json j) override;
};


#endif /* AudioSourceService_hpp */
