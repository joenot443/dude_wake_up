//
//  BlurShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef BlurShader_hpp
#define BlurShader_hpp

#include "ofMain.h"
#include "VideoSettings.h"
#include "ofxImGui.h"
#include "Math.hpp"
#include "Shader.hpp"
#include <stdio.h>

struct BlurShader: Shader {
  BlurSettings *settings;
  ofShader shader;
public:
  BlurShader(BlurSettings *settings) : settings(settings) {}
  void setup() override {
    shader.load("shadersGL2/new/blur");
  }

  void shade(ofFbo *frame, ofFbo *canvas) override {
    canvas->begin();
    shader.begin();
    shader.setUniformTexture("tex", frame->getTexture(), 4);
    shader.setUniform2f("dimensions", frame->getWidth(), frame->getHeight());
    shader.setUniform1i("size", nearestOdd(settings->radius.value));
    shader.setUniform1f("blur_mix", settings->mix.value);
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


#endif /* BlurShader_hpp */
