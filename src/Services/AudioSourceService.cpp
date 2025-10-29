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
    // Only update BPM if not locked
    if (!source->audioAnalysis.bpmLocked->boolValue) {
      source->audioAnalysis.bpm->setValue((int) selectedSampleTrack->bpm);
    }
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
  // Get every input audio device
  auto devices = ofSoundStream().getDeviceList();
  // Create an AudioSource for each input audio device
  for (auto device : devices) {
    // Continue if the AudioSource is not an input device
    if (device.inputChannels == 0) {
      continue;
    }
    
    auto audioSource = std::make_shared<MicrophoneAudioSource>();
    audioSource->device = device;
    audioSource->deviceId = formatString("%d", device.deviceID);
    audioSource->name = device.name;
    // Use deviceId as the stable ID for microphone sources
    audioSource->id = formatString("mic-%s", audioSource->deviceId.c_str());
    audioSourceMap[audioSource->id] = audioSource;
    
    if (device.isDefaultInput) {
      defaultAudioSource = audioSource;
      selectedAudioSource = defaultAudioSource;
    }
  }
  
  auto fileAudioSource = std::make_shared<FileAudioSource>();
  fileAudioSource->id = "file-audio-source";
  fileAudioSource->name = "Sample Track";
  fileAudioSource->track = defaultSampleAudioTrack();
  audioSourceMap[fileAudioSource->id] = fileAudioSource;

  // Add system audio source
  auto systemAudioSource = std::make_shared<SystemAudioSource>();
  systemAudioSource->id = "system-audio-source";
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
  if (!selectedAudioSource->active || !selectedAudioSource->audioAnalysis.bpmEnabled) {
    return;
  }

  // Update BPM value based on mode (if not locked)
  if (!selectedAudioSource->audioAnalysis.bpmLocked->boolValue) {
    if (selectedAudioSource->audioAnalysis.bpmMode == BpmMode_Auto) {
      // Use detected BPM from audio thread
      selectedAudioSource->audioAnalysis.bpm->setValue(selectedAudioSource->lastDetectedBpm);
    }
    else if (selectedAudioSource->audioAnalysis.bpmMode == BpmMode_Link && link != nullptr) {
      // Use Ableton Link BPM
      selectedAudioSource->audioAnalysis.bpm->setValue(link->captureAppSessionState().tempo() * 2.0);
    }
    // Manual mode: don't update BPM, user controls it
  }

  // Sync timing to detected beats in Auto mode
  if (selectedAudioSource->audioAnalysis.bpmMode == BpmMode_Auto) {
    auto timeSinceLastBeat = std::chrono::steady_clock::now() - selectedAudioSource->lastBeatTime;
    auto msSinceLastBeat = std::chrono::duration_cast<std::chrono::milliseconds>(timeSinceLastBeat).count();

    // If we got a beat recently (within last 100ms), sync our timing
    if (msSinceLastBeat < 100) {
      selectedAudioSource->audioAnalysis.bpmStartTime =
        std::chrono::duration_cast<std::chrono::milliseconds>(selectedAudioSource->lastBeatTime.time_since_epoch()).count();
    }
  }

  // Drive the beat pulse based on mode
  if (selectedAudioSource->audioAnalysis.bpmMode == BpmMode_Link && link != nullptr) {
    // Link mode: use Link's beat timing directly
    double beatCount = link->captureAppSessionState().beatAtTime(link->clock().micros(), 4.);
    selectedAudioSource->audioAnalysis.updateBeat(beatCount - floor(beatCount));
  }
  else {
    // Auto/Manual mode: use bpmPct() calculation
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

  // Save the selected audio source ID
  j["selectedAudioSourceId"] = selectedAudioSource->id;

  // Save the active state
  j["active"] = selectedAudioSource->active;

  // Save audio analysis settings (release/scale values)
  j["frequencyRelease"] = selectedAudioSource->audioAnalysis.frequencyRelease->value;
  j["frequencyScale"] = selectedAudioSource->audioAnalysis.frequencyScale->value;
  j["bpmNudge"] = selectedAudioSource->audioAnalysis.bpmNudge->value;
  j["bpmLocked"] = selectedAudioSource->audioAnalysis.bpmLocked->boolValue;
  j["smoothingMode"] = static_cast<int>(selectedAudioSource->audioAnalysis.smoothingMode);
  j["loudnessRelease"] = selectedAudioSource->audioAnalysis.rmsAnalysisParam.release->value;

  return j;
}

void AudioSourceService::loadConfig(json j) {
  // Load by selectedAudioSourceId
  if (j.contains("selectedAudioSourceId")) {
    std::string sourceId = j["selectedAudioSourceId"];
    if (audioSourceMap.count(sourceId) != 0) {
      selectedAudioSource = audioSourceMap[sourceId];
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
  if (j.contains("bpmLocked")) {
    selectedAudioSource->audioAnalysis.bpmLocked->setBoolValue(j["bpmLocked"]);
  }
  if (j.contains("smoothingMode")) {
    selectedAudioSource->audioAnalysis.smoothingMode = static_cast<SmoothingMode>(j["smoothingMode"]);
  }
  if (j.contains("loudnessRelease")) {
    selectedAudioSource->audioAnalysis.rmsAnalysisParam.release->setValue(j["loudnessRelease"]);
  }
}

void AudioSourceService::selectAudioSource(
                                           std::shared_ptr<AudioSource> source) {
  if (selectedAudioSource) { selectedAudioSource->disable(); }
  
  selectedAudioSource = source;
  selectedAudioSource->setup();
}
