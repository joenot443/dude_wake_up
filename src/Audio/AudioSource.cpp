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

void AudioSource::setRecordingCallback(AudioRecordingCallback callback) {
  std::lock_guard<std::mutex> lock(recordingMutex);
  recordingCallback = callback;
}

void AudioSource::clearRecordingCallback() {
  std::lock_guard<std::mutex> lock(recordingMutex);
  recordingCallback = nullptr;
}

void AudioSource::audioIn(ofSoundBuffer &soundBuffer) {
  // Call recording callback first (before silence check) to capture all audio
  {
    std::lock_guard<std::mutex> lock(recordingMutex);
    if (recordingCallback) {
      auto& buffer = soundBuffer.getBuffer();
      recordingCallback(buffer.data(),
                        static_cast<int>(soundBuffer.getNumFrames()),
                        static_cast<int>(soundBuffer.getNumChannels()),
                        soundBuffer.getSampleRate());
    }
  }

  float avg = 0.0f;
  for (size_t i = 0; i < soundBuffer.getNumFrames(); i++) {
    for (size_t c = 0; c < soundBuffer.getNumChannels(); c++) {
      avg += abs(soundBuffer.getSample(i, c));
    }
  }
  avg /= static_cast<float>(soundBuffer.getNumFrames() * soundBuffer.getNumChannels());

  // If we have mostly silence, return (but recording still captured above)
  if (abs(avg) < 0.001f) {
    return;
  }

  auto& buffer = soundBuffer.getBuffer();
  processFrame(buffer);
  debugGist();
}

void AudioSource::processFrame(const std::vector<float>& frame) {
  // Gist expects frames of exactly 512 samples
  // Process the buffer in chunks, calling BTrack for EACH chunk
  const size_t gistFrameSize = 512;

  // Process the buffer in chunks
  for (size_t offset = 0; offset + gistFrameSize <= frame.size(); offset += gistFrameSize) {
    std::vector<float> chunk(frame.begin() + offset, frame.begin() + offset + gistFrameSize);

    gist.processAudioFrame(chunk);
    audioAnalysis.analyzeFrame(&gist);

    // Store waveform data (downsample from 512 to 256 for consistency with spectrum)
    audioAnalysis.waveform.clear();
    audioAnalysis.waveform.reserve(256);
    for (size_t i = 0; i < 256; i++) {
      // Downsample by averaging pairs of samples
      size_t idx = i * 2;
      float avg = (chunk[idx] + chunk[idx + 1]) * 0.5f;
      audioAnalysis.waveform.push_back(avg);
    }

    // Run beat detection only in Auto mode
    // BTrack expects to be called once per hop (512 samples)
    if (audioAnalysis.bpmEnabled && audioAnalysis.bpmMode == BpmMode_Auto) {
      SimpleBeat beat = btrackDetector.processAudioFrame(chunk);

      // Store the detected beat info for use by AudioSourceService
      if (beat.bpm > 0 && beat.bpm >= 50 && beat.bpm <= 200) {
        lastDetectedBpm = beat.bpm;
      }

      if (beat.isBeat) {
        lastBeatTime = std::chrono::steady_clock::now();
      }
    }
  }
}

void AudioSource::debugGist() {
  // Implement debugging logic here if needed
}
