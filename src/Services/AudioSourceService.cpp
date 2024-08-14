//
//  AudioSourceService.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 12/21/22.
//

#include "AudioSourceService.hpp"
#include "LayoutStateService.hpp"
#include "ConfigService.hpp"
#include "UUID.hpp"

void AudioSourceService::setup() {
  ofSoundStream().printDeviceList();
  
  link.enable(true);

  // Get every input audio device
  auto devices = ofSoundStream().getDeviceList();
  // Create an AudioSource for each input audio device
  for (auto device : devices) {
    // Continue if the AudioSource is not an input device
    if (device.inputChannels == 0) {
      continue;
    }

    auto audioSource = std::make_shared<AudioSource>();
    audioSource->id = UUID::generateUUID();
    audioSource->device = device;
    audioSource->deviceId = device.deviceID;
    audioSource->name = device.name;
    audioSourceMap[audioSource->deviceId] = audioSource;
    
    if (device.isDefaultInput) {
      defaultAudioSource = audioSource;
      selectedAudioSource = defaultAudioSource;
    }
  }
}

std::vector<std::shared_ptr<AudioSource>> AudioSourceService::audioSources() {
  std::vector<std::shared_ptr<AudioSource>> audioSources;
  for (auto const &[key, val] : audioSourceMap) {
    audioSources.push_back(val);
  }
  return audioSources;
}

std::shared_ptr<AudioSource>
AudioSourceService::audioSourceForId(std::string id) {
  if (audioSourceMap.count(id) == 0) {
    log("Tried to get AudioSource %s, but it doesn't exist", id.c_str());
    return nullptr;
  }
  return audioSourceMap[id];
}

void AudioSourceService::update() {
  if (LayoutStateService::getService()->abletonLinkEnabled) {
    selectedAudioSource->audioAnalysis.bpmEnabled = true;
    selectedAudioSource->audioAnalysis.bpm->setValue(link.captureAppSessionState().tempo());
    
    double beatCount = link.captureAppSessionState().beatAtTime(link.clock().micros(), 4.);    
    // Only use the fractional component of the beatCount
    selectedAudioSource->audioAnalysis.updateBeat(beatCount - floor(beatCount));
  } else {
    selectedAudioSource->audioAnalysis.bpm->value = AudioSourceService::getService()->tapper.bpm();
    selectedAudioSource->audioAnalysis.updateBeat(tapper.beatPerc());
  }
}

std::shared_ptr<AudioSource>
AudioSourceService::audioSourceForParamId(std::string paramId) {
  if (audioSourceIdToParamId.count(paramId) == 0) {
    log("Tried to get AudioSource for paramId %s, but it doesn't exist",
        paramId.c_str());
    return nullptr;
  }
  return audioSourceMap[audioSourceIdToParamId[paramId]];
}

void AudioSourceService::removeParamMapping(std::string paramId) {
  if (audioSourceIdToParamId.count(paramId) == 0) {
    log("Tried to remove param mapping for paramId %s, but it doesn't exist",
        paramId.c_str());
    return;
  }
  audioSourceIdToParamId.erase(paramId);
}

json AudioSourceService::config() {
  json j;
  
  if (selectedAudioSource != nullptr) {
    j["deviceId"] = selectedAudioSource->deviceId;
    j["active"] = selectedAudioSource->active;
  }
  
  return j;
}

void AudioSourceService::loadConfig(json j) {
  if (j.contains("deviceId")) {
    std::string deviceId = j["deviceId"];
    if (audioSourceMap.count(deviceId) != 0) {
      selectedAudioSource = audioSourceMap[deviceId];
      selectedAudioSource->active = j["active"];
      selectedAudioSource->toggle();
    }
  }
}

void AudioSourceService::selectAudioSource(
    std::shared_ptr<AudioSource> source) {
  if (selectedAudioSource) { selectedAudioSource->disable(); }
  
  selectedAudioSource = source;
  selectedAudioSource->setup();
}
