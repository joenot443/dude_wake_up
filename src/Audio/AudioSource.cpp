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
  // Gist expects frames of exactly 512 samples.
  // The audio callback may deliver any frame count (e.g. file playback at a
  // non-device sample rate often yields <512-frame buffers). Accumulate and
  // slice into fixed 512-sample chunks so the FFT always has full input.
  const size_t gistFrameSize = 512;

  gistAccumulator.insert(gistAccumulator.end(), frame.begin(), frame.end());

  // Guard against unbounded growth if the processor can't keep up.
  const size_t maxAccumulator = gistFrameSize * 8;
  if (gistAccumulator.size() > maxAccumulator) {
    gistAccumulator.erase(gistAccumulator.begin(),
                          gistAccumulator.end() - gistFrameSize);
  }

  while (gistAccumulator.size() >= gistFrameSize) {
    std::vector<float> chunk(gistAccumulator.begin(),
                             gistAccumulator.begin() + gistFrameSize);

    gist.processAudioFrame(chunk);
    audioAnalysis.analyzeFrame(&gist);

    // Store waveform data (downsample from 512 to 256 for consistency with spectrum)
    audioAnalysis.waveform.clear();
    audioAnalysis.waveform.reserve(256);
    for (size_t i = 0; i < 256; i++) {
      size_t idx = i * 2;
      float avg = (chunk[idx] + chunk[idx + 1]) * 0.5f;
      audioAnalysis.waveform.push_back(avg);
      audioAnalysis.renderRawWaveform[i] = avg;
    }
    {
      auto &h = audioAnalysis.renderRawHistory;
      const size_t n = AudioAnalysis::kSpectrumSize;
      std::move(h.begin() + n, h.end(), h.begin());
      std::copy_n(audioAnalysis.renderRawWaveform.begin(), n, h.end() - n);
    }

    // Run beat detection only in Auto mode. BTrack expects one call per 512-sample hop.
    if (audioAnalysis.bpmEnabled && audioAnalysis.bpmMode == BpmMode_Auto) {
      SimpleBeat beat = btrackDetector.processAudioFrame(chunk);

      if (beat.bpm > 0 && beat.bpm >= 50 && beat.bpm <= 200) {
        lastDetectedBpm = beat.bpm;
      }

      if (beat.isBeat) {
        auto now = std::chrono::steady_clock::now();
        auto msSinceLast = std::chrono::duration_cast<std::chrono::milliseconds>(
            now - lastBeatTime).count();
        // Debounce: reject beats within 60% of a beat duration.
        float minIntervalMs = 36000.0f / std::max(1.0f, audioAnalysis.bpm->value);
        if (msSinceLast > minIntervalMs) {
          lastBeatTime = now;
          audioAnalysis.beatCount++;
        }
      }
    }

    gistAccumulator.erase(gistAccumulator.begin(),
                          gistAccumulator.begin() + gistFrameSize);
  }
}

void AudioSource::debugGist() {
  // Implement debugging logic here if needed
}
