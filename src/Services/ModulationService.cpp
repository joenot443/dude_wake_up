//
//  ModulationService.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 2022-07-04.
//

#include "ModulationService.hpp"

void ModulationService::addAudioAnalysis(AudioAnalysis *analysis) {
  audioAnalysis.push_back(analysis);
}

void ModulationService::addMapping(std::shared_ptr<Parameter> videoParam, std::shared_ptr<Parameter> audioParam) {
  mappings[videoParam->name] = ModulationSetting(videoParam, audioParam);
  videoParam->driver = audioParam;
}

void ModulationService::removeMapping(std::shared_ptr<Parameter> videoParam) {
  mappings.erase(videoParam->name);
  videoParam->driver = NULL;
}

void ModulationService::tickMappings() {
  for (auto const& iter : mappings)
  {
    ModulationSetting pair = iter.second;
    pair.videoParam->value = pair.audioParam->value;
  }
}

bool ModulationService::videoParameterIsBeingDriven(std::shared_ptr<Parameter> videoParam) {
  return mappings.count(videoParam->name);
}

std::shared_ptr<Parameter> ModulationService::audioParameterDriving(std::shared_ptr<Parameter> videoParam) {
  if (mappings.count(videoParam->name)) {
    return mappings[videoParam->name].audioParam;
  }
  return NULL;
}