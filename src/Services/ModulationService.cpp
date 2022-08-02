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

void ModulationService::addMapping(Parameter *videoParam, Parameter *audioParam) {
  mappings[videoParam->name] = ModulationSetting(videoParam, audioParam);
  videoParam->driver = audioParam;
}

void ModulationService::removeMapping(Parameter *videoParam) {
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

bool ModulationService::videoParameterIsBeingDriven(Parameter *videoParam) {
  return mappings.count(videoParam->name);
}

Parameter* ModulationService::audioParameterDriving(Parameter *videoParam) {
  if (mappings.count(videoParam->name)) {
    return mappings[videoParam->name].audioParam;
  }
  return NULL;
}
