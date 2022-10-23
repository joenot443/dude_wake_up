//
//  EmptyShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef EmptyShader_hpp
#define EmptyShader_hpp

#include "ofMain.h"
#include "VideoSettings.hpp"
#include "ofxImGui.h"
#include "Shader.hpp"
#include <stdio.h>

struct EmptyShader: Shader {
  ofShader shader;
  void setup() override {
    shader.load("shadersGL2/new/empty");

  }

  void shade(ofFbo *frame, ofFbo *canvas) override {
    canvas->begin();
    shader.begin();
    shader.setUniformTexture("tex", frame->getTexture(), 4);
    frame->draw(0, 0);
    shader.end();
    canvas->end();
  }

  void clear() override {
    
  }
};

#endif /* EmptyShader_hpp */
