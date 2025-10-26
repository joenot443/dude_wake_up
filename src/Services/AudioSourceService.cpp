//
//  AudioSourceService.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 12/21/22.
//

#include "AudioSourceService.hpp"
#include "LayoutStateService.hpp"
#include "FileAudioSource.hpp"
#include "Console.hpp"
#include "MicrophoneAudioSource.hpp"
#include "SystemAudioSource.hpp"
#include "ConfigService.hpp"
#include "UUID.hpp"

void AudioSourceService::setup() {
  populateTracks();
  populateSources();
}

void AudioSourceService::setupAbleton() {
  if (link != nullptr) return;
  
  link = std::make_shared<ableton::Link>(120);
  link->enable(true);
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
    // Remove the first 3 characters from the name
    std::string trackName = name.substr(3);

    // Trim the extension from the name
    trackName = trackName.substr(0, trackName.find_last_of('.'));
    
    audioTracks.push_back(std::make_shared<AudioTrack>(trackName, path, bpm));
  }
  // Sort by name
  std::sort(audioTracks.begin(), audioTracks.end(), [](std::shared_ptr<AudioTrack> a, std::shared_ptr<AudioTrack> b)
            { return a->name < b->name; });
  sampleTracks = audioTracks;
  selectedSampleTrack = audioTracks.front();
}

void AudioSourceService::populateSources() {
  if (LayoutStateService::getService()->abletonLinkEnabled) {
    link->enable(true);
  }
  
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
    audioSource->deviceId = formatString("%d", device.deviceID);
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
  
  // Add system audio source
  auto systemAudioSource = std::make_shared<SystemAudioSource>();
  systemAudioSource->id = UUID::generateUUID();
  systemAudioSource->name = "System Audio";
  audioSourceMap[systemAudioSource->id] = systemAudioSource;
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


  // Link Case
  if (LayoutStateService::getService()->abletonLinkEnabled && link != nullptr) {
    auto state = link->captureAppSessionState();
    selectedAudioSource->audioAnalysis.bpm->setValue(link->captureAppSessionState().tempo() * 2.0);

    // Drive the beat with Ableton
    if (selectedAudioSource->audioAnalysis.bpmEnabled) {
      double beatCount = link->captureAppSessionState().beatAtTime(link->clock().micros(), 4.);
      selectedAudioSource->audioAnalysis.updateBeat(beatCount - floor(beatCount));
    }
  }
  // Auto case
  else if ((selectedAudioSource->type() == AudioSourceType_Microphone || selectedAudioSource->type() == AudioSourceType_System) && selectedAudioSource->audioAnalysis.bpmEnabled) {
    // Only update the BPM from the detector if we're in Auto mode
    if (selectedAudioSource->audioAnalysis.bpmMode == BpmMode_Auto) {
      selectedAudioSource->audioAnalysis.bpm->value = AudioSourceService::getService()->selectedAudioSource->btrackDetector.getCurrentBpm();
    }

    // Drive the beat normally
    if (selectedAudioSource->audioAnalysis.bpmEnabled) {
      selectedAudioSource->audioAnalysis.updateBeat(selectedAudioSource->audioAnalysis.bpmPct());
    }
  }
  // File case
  else if (selectedAudioSource->type() == AudioSourceType_File) {

    // Drive the beat normally
    if (selectedAudioSource->audioAnalysis.bpmEnabled) {
      selectedAudioSource->audioAnalysis.updateBeat(selectedAudioSource->audioAnalysis.bpmPct());
    }
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

  // Save the selected audio source ID
  j["selectedAudioSourceId"] = selectedAudioSource->id;

  // Save the active state
  j["active"] = selectedAudioSource->active;

  // Save audio analysis settings (release/scale values)
  j["frequencyRelease"] = selectedAudioSource->audioAnalysis.frequencyRelease->value;
  j["frequencyScale"] = selectedAudioSource->audioAnalysis.frequencyScale->value;
  j["bpmNudge"] = selectedAudioSource->audioAnalysis.bpmNudge->value;
  j["smoothingMode"] = static_cast<int>(selectedAudioSource->audioAnalysis.smoothingMode);
  j["loudnessRelease"] = selectedAudioSource->audioAnalysis.rmsAnalysisParam.release->value;

  // For backward compatibility with MicrophoneAudioSource
  std::shared_ptr<MicrophoneAudioSource> audioSource = std::dynamic_pointer_cast<MicrophoneAudioSource>(selectedAudioSource);
  if (audioSource != nullptr) {
    j["deviceId"] = audioSource->deviceId;
  }

  return j;
}

void AudioSourceService::loadConfig(json j) {
  // Try to load by selectedAudioSourceId first (new format)
  if (j.contains("selectedAudioSourceId")) {
    std::string sourceId = j["selectedAudioSourceId"];
    if (audioSourceMap.count(sourceId) != 0) {
      selectedAudioSource = audioSourceMap[sourceId];
    }
  }
  // Fall back to deviceId (backward compatibility)
  else if (j.contains("deviceId")) {
    std::string deviceId = j["deviceId"];
    if (audioSourceMap.count(deviceId) != 0) {
      selectedAudioSource = audioSourceMap[deviceId];
    }
  }

  // Restore the active state
  if (j.contains("active")) {
    bool wasActive = j["active"];
    // Don't set active first - setup() will set it to true
    if (wasActive) {
      selectedAudioSource->setup();
    }
  }

  // Restore audio analysis settings (release/scale values)
  if (j.contains("frequencyRelease")) {
    selectedAudioSource->audioAnalysis.frequencyRelease->setValue(j["frequencyRelease"]);
  }
  if (j.contains("frequencyScale")) {
    selectedAudioSource->audioAnalysis.frequencyScale->setValue(j["frequencyScale"]);
  }
  if (j.contains("bpmNudge")) {
    selectedAudioSource->audioAnalysis.bpmNudge->setValue(j["bpmNudge"]);
  }
  if (j.contains("smoothingMode")) {
    selectedAudioSource->audioAnalysis.smoothingMode = static_cast<SmoothingMode>(j["smoothingMode"]);
  }
  if (j.contains("loudnessRelease")) {
    selectedAudioSource->audioAnalysis.rmsAnalysisParam.release->setValue(j["loudnessRelease"]);
  }

  if (LayoutStateService::getService()->abletonLinkEnabled) {
    setupAbleton();
  }
}

void AudioSourceService::selectAudioSource(
                                           std::shared_ptr<AudioSource> source) {
  if (selectedAudioSource) { selectedAudioSource->disable(); }
  
  selectedAudioSource = source;
  selectedAudioSource->setup();
}
