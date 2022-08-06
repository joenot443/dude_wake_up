//
//  ofApp.h
//  dude_wake_up
//
//  Created by Joe Crozier on 2022-07-23.
//

#ifndef ofApp_h
#define ofApp_h
#include "ofBaseApp.h"
#include <stdio.h>
#include "ofMain.h"
#include "ofxImGui.h"

class ofApp : public ofBaseApp{
  
public:
  ofApp() : v(0) {}
  
  void setup() {
    gui.setup(nullptr, false, ImGuiConfigFlags_ViewportsEnable);
  }
  void draw() {
    gui.begin();
    
    
    gui.end();
  }
  
private:
  ofxImGui::Gui gui;
  float v;
};


#endif /* ofApp_h */
