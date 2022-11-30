#pragma once

#include "ofMain.h"
#include "ofxImGui.h"
#include "VideoSettingsView.hpp"
#include "MainSettingsView.hpp"
#include "AudioSettingsView.hpp"
#include "MainStageView.hpp"
#include "VideoStream.hpp"
#include "AudioStream.hpp"

class MainApp : public ofBaseApp {
  
public:
  void setup();
  void update();
  void draw();
  
  MainApp(std::shared_ptr<ofAppBaseWindow> window) : window(window) {};
  
  // Public

private:
  void drawStream(ofEventArgs & args);
  void exitStream(ofEventArgs & args);
  void dragEvent(ofDragInfo dragInfo);
  
  void drawMainStage();

  void drawAudioSettings();
  void drawVideoSettings();
  void drawMainSettings();
  void completeFrame();
  void pushAudioStream(std::shared_ptr<AudioStreamConfig> configToPush);
  void pushVideoStream(std::shared_ptr<StreamConfig> configToPush);
  void removeVideoStream(int streamId);
  void removeAudioStream(int streamId);
  void resetState();
  
  
  ofxImGui::Gui gui;
  MainSettingsView *mainSettingsView;
  MainStageView *mainStageView = new MainStageView();
  // Config which will be pushed at the end of the frame.
  std::shared_ptr<StreamConfig> configToPush = NULL;
  std::shared_ptr<AudioStreamConfig> audioConfigToPush = NULL;
  std::shared_ptr<ofAppBaseWindow> window;
  std::vector<shared_ptr<ofAppBaseWindow> > windows;
  std::vector<VideoStream*> videoStreams;
  std::vector<VideoSettingsView*> videoSettingsViews;
  std::vector<AudioStream*> audioStreams;
  std::vector<AudioSettingsView*> audioSettingsViews;
  
  int streamDrawIndex;
  bool isSetup = false;
};
