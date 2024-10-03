//
//  MicrophoneAudioSource.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 9/21/24.
//

#ifndef MicrophoneAudioSource_h
#define MicrophoneAudioSource_h

#include "AudioSource.hpp"

class MicrophoneAudioSource : public AudioSource {
public:
  std::string deviceId;
  ofSoundDevice device;
  ofSoundStream stream;
  
  void setup() override;
  void toggle() override;
  void disable() override;
  void audioIn(ofSoundBuffer &soundBuffer) override;
  AudioSourceType type() override { return AudioSourceType_Microphone; }
};

#endif /* MicrophoneAudioSource_h */
