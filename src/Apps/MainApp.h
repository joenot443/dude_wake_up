#pragma once

#include "ofMain.h"
#include "ofxImGui.h"
#include "MainStageView.hpp"

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

  void drawMainSettings();
  void resetState();
  
  std::shared_ptr<ofAppBaseWindow> window;
  ofxImGui::Gui gui;
  MainStageView *mainStageView = new MainStageView();
  int streamDrawIndex;
  bool isSetup = false;
};
