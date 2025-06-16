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
#include "WaveformOscillator.hpp"
#include "Parameter.hpp"
#include "Shader.hpp"
#include <stdio.h>

struct OverlaySettings: public ShaderSettings {
  std::shared_ptr<Parameter> flip;
  std::shared_ptr<Parameter> alpha;

  std::shared_ptr<WaveformOscillator> alphaOscillator;
  
  OverlaySettings(std::string shaderId, json j) :
  flip(std::make_shared<Parameter>("Flip", 0.0, 0.0, 1.0, ParameterType_Bool)),
  alpha(std::make_shared<Parameter>("Alpha", 1.0, 0.0, 1.0)),
  alphaOscillator(std::make_shared<WaveformOscillator>(alpha)),
  ShaderSettings(shaderId, j, "Overlay") {
    parameters = { flip, alpha };
    oscillators = { alphaOscillator };
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
    
    if (settings->flip->boolValue && hasInputAtSlot(InputSlotTwo)) {
      // Draw the second input first, then the main frame on top
      std::shared_ptr<ofFbo> tex2 = inputAtSlot(InputSlotTwo)->frame();
      tex2->draw(0, 0);
      ofSetColor(255, 255, 255, settings->alpha->value * 255);
      frame->draw(0, 0);
    } else {
      // Default behavior: draw main frame first, then second input on top
      frame->draw(0, 0);
      if (hasInputAtSlot(InputSlotTwo)) {
        std::shared_ptr<ofFbo> tex2 = inputAtSlot(InputSlotTwo)->frame();
        ofSetColor(255, 255, 255, settings->alpha->value * 255);
        tex2->draw(0, 0);
      }
    }
    
    canvas->end();
    ofSetColor(255, 255, 255, 255); // Reset color
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
    CommonViews::ShaderCheckbox(settings->flip);
    CommonViews::ShaderParameter(settings->alpha, settings->alphaOscillator);
  }
};

#endif
