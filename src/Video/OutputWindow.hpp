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
#include "ShaderChainer.hpp"

struct OutputWindow : public ofBaseApp
{
  OutputWindow(std::shared_ptr<ofFbo> fbo) : fbo(fbo)
  { };

  std::shared_ptr<ofFbo> fbo;
  
  void setSource(std::shared_ptr<ofFbo> fbo);
  void setup();
  void update();
  void draw();
  void teardown();
};

#endif /* OutputWindow_hpp */
