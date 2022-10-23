//
//  MainSettings.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 2022-05-15.
//

#ifndef MainSettings_h
#define MainSettings_h
#include "VideoSettings.hpp"
#include "AudioSettings.hpp"
#include <string>

struct MainSettings {
  int activeStreams = 0;
  VideoSource selectedVideoSource = VideoSource_webcam;
  std::string selectedVideoPath = "";
  AudioSource selectedAudioSource = AudioSource_none;
  std::string selectedAudioPath = "";
  int selectedVideoDeviceIndex = 0;
  int selectedAudioDeviceIndex = 0;
  
  
  // Whether the provided settings are valid and we display the Add Video Stream button
  bool canLaunchVideo() {
    switch (selectedVideoSource) {
      case VideoSource_webcam:
        return true;
      
      case VideoSource_file: {
        if (selectedVideoPath.length() > 0) {
          return true;
        }
      }
    }
    
    return false;
  }
  
  // Whether the provided settings are valid and we display the Add Audio Stream button
  bool canLaunchAudio() {
    switch (selectedAudioSource) {
      case AudioSource_none:
        return false;
      case AudioSource_microphone:
        return true;
      case AudioSource_file: {
        if (selectedAudioPath.length() > 0) {
          return true;
        }
      }
    }
    
    return false;
  }
};

struct AudioStreamConfig {
  AudioSource source;
  std::string path;
  int index;
  
  AudioStreamConfig(AudioSource source, std::string path, int index) : source(source), path(path), index(index) {}

};

struct StreamConfig {
  VideoSource source;
  std::string path;
  int index;
  std::string streamId;
  
  StreamConfig(VideoSource source, std::string path, int index) : source(source), path(path), index(index),
  streamId(formatString("%d-stream", index))
  {}
};

#endif /* MainSettings_h */
