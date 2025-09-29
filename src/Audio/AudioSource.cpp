//
//  AudioSource.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 12/23/22.
//

#include "AudioSource.hpp"
#include "Console.hpp"
#include "AudioSourceService.hpp"
#include "Gist.h"
#include <stdio.h>
#include <chrono>

void AudioSource::toggle() {
  if (active)
    disable();
  else
    setup();
}

void AudioSource::audioIn(ofSoundBuffer &soundBuffer) {
  float avg = 0.0f;
  for (size_t i = 0; i < soundBuffer.getNumFrames(); i++) {
    for (size_t c = 0; c < soundBuffer.getNumChannels(); c++) {
      avg += abs(soundBuffer.getSample(i, c));
    }
  }
  avg /= static_cast<float>(soundBuffer.getNumFrames() * soundBuffer.getNumChannels());
  
  // If we have mostly silence, return
  if (abs(avg) < 0.001f) {
    return;
  }
  
  auto& buffer = soundBuffer.getBuffer();
  processFrame(buffer);
  debugGist();
}

void AudioSource::processFrame(const std::vector<float>& frame) {
  gist.processAudioFrame(frame);
  audioAnalysis.analyzeFrame(&gist);
  
  // Process beat detection using BTrack only in Auto mode
  if (audioAnalysis.bpmMode == BpmMode_Auto) {
    SimpleBeat beat = btrackDetector.processFrame(&gist);
    
    // Update BPM if we have a valid detection
    if (beat.bpm > 0 && beat.bpm >= 50 && beat.bpm <= 200) {
      audioAnalysis.bpm->setValue(beat.bpm);
    }
    
    // If beat detected, reset timing for beat synchronization
    if (beat.isBeat) {
      auto currentTime = std::chrono::steady_clock::now();
      audioAnalysis.bpmStartTime = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime.time_since_epoch()).count();
    }
  }
}

void AudioSource::debugGist() {
  // Implement debugging logic here if needed
}
