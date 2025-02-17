//
//  AudioSourceService.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 12/21/22.
//

#include "AudioSourceService.hpp"
#include "LayoutStateService.hpp"
#include "FileAudioSource.hpp"
#include "MicrophoneAudioSource.hpp"
#include "ConfigService.hpp"
#include "UUID.hpp"

void AudioSourceService::setup() {
  populateTracks();
  populateSources();
}

void AudioSourceService::affirmSampleAudioTrack() {
  selectedSampleTrack = sampleTracks[selectedSampleTrackParam->intValue];
  
  // Load the track into the Source
  if (selectedAudioSource->type() == AudioSourceType_File) {
    std::shared_ptr<FileAudioSource> source = std::dynamic_pointer_cast<FileAudioSource>(selectedAudioSource);
    source->audioAnalysis.bpm->setValue((int) selectedSampleTrack->bpm);
    source->loadFile(selectedSampleTrack);
  }
}

std::shared_ptr<AudioTrack> AudioSourceService::defaultSampleAudioTrack() {
  return sampleTracks.front();
}

void AudioSourceService::populateTracks() {
  std::vector<std::shared_ptr<AudioTrack>> audioTracks;

  ofDirectory dir(ofToDataPath("audio/"));
  dir.listDir();
  for (int i = 0; i < dir.size(); i++)
  {
    std::string path = dir.getPath(i);
    std::string name = dir.getName(i);
    // The first 3 characters of the filename will correspond to the BPM of the track.
    // i.e. 120A Nottawa Song.mp3
    int bpm = std::stoi(name.substr(0, 3));
    if (bpm == 0) {
      continue;
    }
    std::string trackName = name.substr(4);
    audioTracks.push_back(std::make_shared<AudioTrack>(trackName, path, bpm));
  }
  // Sort by name
  std::sort(audioTracks.begin(), audioTracks.end(), [](std::shared_ptr<AudioTrack> a, std::shared_ptr<AudioTrack> b)
            { return a->name < b->name; });
  sampleTracks = audioTracks;
  selectedSampleTrack = audioTracks.front();
}

void AudioSourceService::populateSources() {
  link.enable(true);
  
  // Get every input audio device
  auto devices = ofSoundStream().getDeviceList();
  // Create an AudioSource for each input audio device
  for (auto device : devices) {
    // Continue if the AudioSource is not an input device
    if (device.inputChannels == 0) {
      continue;
    }
    
    auto audioSource = std::make_shared<MicrophoneAudioSource>();
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
  
  auto fileAudioSource = std::make_shared<FileAudioSource>();
  fileAudioSource->id = UUID::generateUUID();
  fileAudioSource->name = "Sample Track";
  fileAudioSource->track = defaultSampleAudioTrack();
  audioSourceMap[fileAudioSource->id] = fileAudioSource;
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
  // Return if we're not enabled
  if (!selectedAudioSource->active) return;
  
  if (LayoutStateService::getService()->abletonLinkEnabled) {
    selectedAudioSource->audioAnalysis.bpmEnabled = true;
    selectedAudioSource->audioAnalysis.bpm->setValue(link.captureAppSessionState().tempo());
    
    double beatCount = link.captureAppSessionState().beatAtTime(link.clock().micros(), 4.);
    // Only use the fractional component of the beatCount
    selectedAudioSource->audioAnalysis.updateBeat(beatCount - floor(beatCount));
  } else if (selectedAudioSource->type() == AudioSourceType_Microphone){ // Don't update for the samples
    selectedAudioSource->audioAnalysis.bpm->value = AudioSourceService::getService()->tapper.bpm();
    selectedAudioSource->audioAnalysis.updateBeat(tapper.beatPerc());
  } else if (selectedAudioSource->type() == AudioSourceType_File) {
    selectedAudioSource->audioAnalysis.bpmEnabled = true;
    selectedAudioSource->audioAnalysis.updateBeat(selectedAudioSource->audioAnalysis.bpmPct());
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
  std::shared_ptr<MicrophoneAudioSource> audioSource = std::dynamic_pointer_cast<MicrophoneAudioSource>(selectedAudioSource);
  if (audioSource != nullptr) {
    j["deviceId"] = audioSource->deviceId;
    j["active"] = audioSource->active;
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
