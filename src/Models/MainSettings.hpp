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
#include "VideoSource.hpp"
#include <string>

struct MainSettings {
  int activeStreams = 0;
  VideoSourceType selectedVideoSource = VideoSource_webcam;
  std::string selectedVideoPath = "";
  AudioSource selectedAudioSource = AudioSource_none;
  std::string selectedAudioPath = "";
  std::string selectedVideoDevice = "";
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
  VideoSourceType type;
  std::string path;
  std::string name;
  int index;
  std::string streamId;
  
  StreamConfig(VideoSourceType type, std::string path, std::string name, int index) : type(type), path(path), index(index), name(name), streamId(formatString("%d-stream", index)) {}
};

#endif /* MainSettings_h */
