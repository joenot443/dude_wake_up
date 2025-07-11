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
#include "AudioTrack.hpp"
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
  std::shared_ptr<AudioTrack> selectedSampleTrack;
  std::shared_ptr<Parameter> selectedSampleTrackParam;
  std::shared_ptr<ableton::Link> link;

  AudioSourceService() : selectedSampleTrackParam(std::make_shared<Parameter>("Sample Track", ParameterType_Int))
  {};
  
  static AudioSourceService *getService() {
    if (!service) {
      service = new AudioSourceService;
      service->setup();
    }
    return service;
  }
  void setup();
  void populateTracks();
  void populateSources();
  void selectAudioSource(std::shared_ptr<AudioSource> source);
  void affirmSampleAudioTrack();
  
  void update();

  msa::BPMTapper tapper;
  std::vector<std::shared_ptr<AudioTrack>> sampleTracks;

  std::vector<std::shared_ptr<AudioSource>> audioSources();
  std::shared_ptr<AudioTrack> defaultSampleAudioTrack();
  std::shared_ptr<AudioSource> audioSourceForId(std::string id);
  std::shared_ptr<AudioSource> audioSourceForParamId(std::string paramId);

  void removeParamMapping(std::string paramId);
  
  void setupAbleton();

  json config() override;
  void loadConfig(json j) override;
};

#endif /* AudioSourceService_hpp */
