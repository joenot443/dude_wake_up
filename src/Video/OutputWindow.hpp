//
//  OutputWindow.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 11/30/22.
//

#ifndef OutputWindow_hpp
#define OutputWindow_hpp

#include <stdio.h>
#include "ofMain.h"
#include "ofxImGui.h"
#include "Connection.hpp"

struct OutputWindow : public ofBaseApp, std::enable_shared_from_this<OutputWindow>
{
  OutputWindow(std::shared_ptr<ofFbo> fbo, std::shared_ptr<Connectable> connectable) : fbo(fbo), connectable(connectable)
  { };

  void setup();
  void update();
  void draw();
  void teardown();
  void keyReleased(int key);
  
public:
  std::shared_ptr<ofFbo> fbo;
  std::shared_ptr<Connectable> connectable;
};

#endif /* OutputWindow_hpp */
