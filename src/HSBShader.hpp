//
//  HSBShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef HSBShader_hpp
#define HSBShader_hpp

#include "ofMain.h"
#include "VideoSettings.h"
#include "ofxImGui.h"
#include "Shader.hpp"
#include <stdio.h>

struct HSBShader: Shader {
  HSBSettings *settings;
  ofShader shader;
public:
  HSBShader(HSBSettings *settings) : settings(settings) {}
  void setup() override {
    shader.load("shadersGL2/new/hsb");
  }

  void shade(ofFbo *frame, ofFbo *canvas) override {
    canvas->begin();
    shader.begin();
    shader.setUniformTexture("tex", frame->getTexture(), 4);
    shader.setUniform3f("hsbScalar", settings->hue.value, settings->saturation.value, settings->brightness.value);
    shader.setUniform3f("hsbFloor", 0.3, 0.3, 0.3);
    shader.setUniform3f("hsbCeil", 1.0, 1.0, 1.0);
    frame->draw(0, 0);
    shader.end();
    canvas->end();
  }

  void clear() override {
    
  }
  
  bool enabled() override {
    return true;
  }
};

#endif /* HSBShader_hpp */
