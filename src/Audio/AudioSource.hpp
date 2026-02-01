//
//  AudioSource.h
//  dude_wake_up
//
//  Created by Joe Crozier on 12/21/22.
//

#ifndef AudioSource_h
#define AudioSource_h

#include "json.hpp"
#include "ofMain.h"
#include "Gist.h"
#include "AudioSettings.hpp"
#include "BTrackDetector.hpp"
#include <chrono>
#include <functional>
#include <mutex>

using json = nlohmann::json;

// Callback type for audio recording
// Parameters: samples (interleaved), numFrames, numChannels, sampleRate
using AudioRecordingCallback = std::function<void(const float*, int, int, float)>;

enum AudioSourceType {
  AudioSourceType_Microphone,
  AudioSourceType_File,
  AudioSourceType_System
};

class AudioSource {
public:
  std::string id;
  std::string name;
  bool active = false;
  AudioAnalysis audioAnalysis;
  Gist<float> gist = Gist<float>(512, 44100);
  BTrackDetector btrackDetector;

  // Beat detection results (updated by audio thread, read by main thread)
  float lastDetectedBpm = 120.0f;
  std::chrono::steady_clock::time_point lastBeatTime;

  // Audio recording callback (called from audio thread)
  AudioRecordingCallback recordingCallback;
  std::mutex recordingMutex;

  // Set callback to receive raw audio samples during recording
  void setRecordingCallback(AudioRecordingCallback callback);
  void clearRecordingCallback();

  virtual ~AudioSource() = default;
  
  virtual json serialize() { return json(); }
  
  virtual void load(json j) {}
  
  virtual AudioSourceType type() = 0;
  virtual float getSampleRate() const { return 44100.0f; } // Default sample rate

  virtual void setup() = 0;
  virtual void toggle();
  virtual void disable() = 0;
  virtual void audioIn(ofSoundBuffer &soundBuffer);
  virtual void processFrame(const std::vector<float>& frame);
  virtual void debugGist();
};

#endif /* AudioSource_h */
