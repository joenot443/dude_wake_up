//
//  FileAudioSource.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 9/21/24.
//

#ifndef FileAudioSource_h
#define FileAudioSource_h

#include "AudioSource.hpp"
#include "AudioTrack.hpp"
#include "ofxAudioFile.h"
#include <mutex>
#include <atomic>

class FileAudioSource : public AudioSource {
public:
  std::shared_ptr<AudioTrack> track;
  
  ofxAudioFile audioFile;
  uint64_t currentSample = 0;
  size_t frameSize = 512;
  ofSoundStream soundStream;
  
  void loadFile(std::shared_ptr<AudioTrack> audioTrack);
  void setup() override;
  void disable() override;
  
  void audioOut(ofSoundBuffer &outputBuffer);
  
  AudioSourceType type() override { return AudioSourceType_File; }
  
  // Playback control methods
  void pausePlayback();
  void resumePlayback();
  
  // New members
  std::atomic<bool> isPaused{false}; // Pause flag
  
  bool loop = true;                  // Control looping
  
  // Playback position methods
  float getCurrentPlaybackTime() const;
  float getTotalDuration() const;
  float getPlaybackPosition() const;
  void setPlaybackPosition(float position);
  
  // Destructor
  ~FileAudioSource();
  
private:
  mutable std::mutex audioMutex; // To protect shared data
};

#endif /* FileAudioSource_h */
