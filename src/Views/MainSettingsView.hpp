//
//  MainSettingsView.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 2022-05-15.
//

#ifndef MainSettingsView_hpp
#define MainSettingsView_hpp

#include <stdio.h>
#include <functional>
#include <string>
#include "MainSettings.h"
#include "CommonViews.hpp"

struct MainSettingsView {
  void setup();
  void update();
  void draw();
  
  std::function<void(StreamConfig)> addVideoStream;
  std::function<void(AudioStreamConfig)> addAudioStream;
  
  MainSettingsView(MainSettings *mainSettings, std::function<void(StreamConfig)> addVideoStream, std::function<void(AudioStreamConfig)> addAudioStream) : mainSettings(mainSettings), addVideoStream(addVideoStream), addAudioStream(addAudioStream) {};
  
  MainSettings *mainSettings;
  
private:
  std::vector<std::string> videoDeviceNames;
  std::vector<std::string> audioDeviceNames;
  
  void drawVideoSettings();
  void drawAudioSettings();
};

#endif /* MainSettingsView_hpp */
