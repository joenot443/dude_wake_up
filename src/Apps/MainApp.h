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
  void keyPressed(int key);
  void keyReleased(int key);

  void drawMainStage();
  void setupStyle();
  void drawMainSettings();
  void resetState();
  void affirmWindowSize();

  std::shared_ptr<ofAppBaseWindow> window;
  ofxImGui::Gui gui;
  MainStageView *mainStageView = new MainStageView();
  int streamDrawIndex;
  bool isSetup = false;
  

  
  // Threading
  
  std::queue<std::function<void()>> mainThreadTasks;
  std::mutex mainThreadTasksMutex;

  struct Style {
    float alpha = 1.0f;
    float windowPadding[2] = {8.0f, 8.0f};
    float framePadding[2] = {8.0f, 4.0f};
    float itemSpacing[2] = {8.0f, 4.0f};
    
//    ImColor majorColor = ImColor(0.7, 0.8, 0.7);
  };

  Style style;
};

