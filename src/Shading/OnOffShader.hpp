//
//  OnOffShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef OnOffShader_hpp
#define OnOffShader_hpp

#include "ofMain.h"
#include "ShaderSettings.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "WaveformOscillator.hpp"
#include "Parameter.hpp"
#include "Shader.hpp"
#include <stdio.h>

struct OnOffSettings: public ShaderSettings {
  std::shared_ptr<Parameter> onOff;

  OnOffSettings(std::string shaderId, json j) :
  onOff(std::make_shared<Parameter>("Enabled", ParameterType_Bool)),
  ShaderSettings(shaderId, j, "On Off") {
    parameters = { onOff };
    load(j);
    registerParameters();
  };
};

struct OnOffShader: Shader {
  OnOffSettings *settings;
  OnOffShader(OnOffSettings *settings) : settings(settings), Shader(settings) {};

  
  void setup() override {
  }

  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override {
    canvas->begin();
    if (settings->onOff->boolValue) {
      frame->draw(0, 0);
    }
    else {
      ofClear(0,0,0, 255);
      ofClear(0,0,0, 0);
    }
    canvas->end();
  }

  void clear() override {

  }

  int inputCount() override {
    return 1;
  }

  ShaderType type() override {
    return ShaderTypeOnOff;
  }

  void drawSettings() override {
    CommonViews::H3Title("On Off");

    CommonViews::ShaderCheckbox(settings->onOff);
  }
};

#endif
