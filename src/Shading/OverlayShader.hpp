//
//  OverlayShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef OverlayShader_hpp
#define OverlayShader_hpp

#include "ofMain.h"
#include "ShaderSettings.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "ValueOscillator.hpp"
#include "Parameter.hpp"
#include "Shader.hpp"
#include <stdio.h>

struct OverlaySettings: public ShaderSettings {
  OverlaySettings(std::string shaderId, json j) :
  ShaderSettings(shaderId, j, "Overlay") {
    parameters = { };
    oscillators = { };
    load(j);
    registerParameters();
  };
};

struct OverlayShader: Shader {
  OverlaySettings *settings;
  OverlayShader(OverlaySettings *settings) : settings(settings), Shader(settings) {};

  void setup() override {
  }

  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override {
    canvas->begin();
    frame->draw(0, 0);
    if (hasInputAtSlot(InputSlotTwo)) {
      std::shared_ptr<ofFbo> tex2 = inputAtSlot(InputSlotTwo)->frame();
      tex2->draw(0, 0);
    }
    canvas->end();
  }

  void clear() override {
    
  }

  int inputCount() override {
    return 2;
  }

  ShaderType type() override {
    return ShaderTypeOverlay;
  }

  void drawSettings() override {
    CommonViews::H3Title("Overlay");
  }
};

#endif
