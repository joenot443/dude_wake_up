#pragma once

#include "MainStageView.hpp"
#include "ofMain.h"
#include "ofxImGui.h"

class MainApp : public ofBaseApp {

public:
  
  static MainApp *app;
  MainApp(){};

  static MainApp *getApp()
  {
    return app;
  }
  
  void setup();
  void update();
  void draw();
  void executeOnMainThread(const std::function<void()>& task);
  void runMainThreadTasks();
  
  MainApp(std::shared_ptr<ofAppBaseWindow> window) : window(window){
    app = this;
  };

  // Public

private:
  void drawStream(ofEventArgs &args);
  void exitStream(ofEventArgs &args);
  void dragEvent(ofDragInfo dragInfo);
  void keyReleased(int key);

  void drawMainStage();
  void setupStyle();
  void drawMainSettings();
  void resetState();


  std::shared_ptr<ofAppBaseWindow> window;
  ofxImGui::Gui gui;
  MainStageView *mainStageView = new MainStageView();
  int streamDrawIndex;
  bool isSetup = false;
  
  // Threading
  
  std::queue<std::function<void()>> mainThreadTasks;
  std::mutex mainThreadTasksMutex;
};

