//
//  VideoFeedSettingsView.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 2022-05-09.
//

#ifndef VideoFeedSettingsView_hpp
#define VideoFeedSettingsView_hpp

#include <stdio.h>
#include "VideoSettings.h"
#include "VideoStream.h"
#include "BasicSettingsView.hpp"
#include "FeedbackSettingsView.hpp"

struct VideoSettingsView {
public:
  VideoStream *videoStream;
  void setup();
  void update();
  void draw();
  void teardown();
  
  VideoSettings *videoSettings;
  BasicSettingsView basicSettingsView;
  FeedbackSettingsView feedback0SettingsView;
  FeedbackSettingsView feedback1SettingsView;
  FeedbackSettingsView feedback2SettingsView;
  
  std::function<void(int)> closeStream;
  std::vector<std::string> sourceNames;
  
  VideoSettingsView(VideoSettings *videoSettings, VideoStream *videoStream, std::function<void(int)> closeStream)
  : videoSettings(videoSettings), closeStream(closeStream), videoStream(videoStream) {};
  
private:
  void styleWindow();
};

#endif /* VideoFeedSettingsView_hpp */
