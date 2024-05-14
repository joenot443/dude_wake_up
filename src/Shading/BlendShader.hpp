//
//  BlendShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef BlendShader_hpp
#define BlendShader_hpp

#include "ofMain.h"
#include "ShaderSettings.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "ValueOscillator.hpp"
#include "Parameter.hpp"
#include "Shader.hpp"
#include <stdio.h>

struct BlendSettings: public ShaderSettings {
  std::shared_ptr<Parameter> mode;
  std::shared_ptr<Parameter> blendWithEmpty;
  std::shared_ptr<Parameter> flip;
  std::shared_ptr<Parameter> alpha;
  
  std::shared_ptr<WaveformOscillator> alphaOscillator;

  BlendSettings(std::string shaderId, json j) :
  mode(std::make_shared<Parameter>("Mode", 0.0, 0.0, 13.0)),
  blendWithEmpty(std::make_shared<Parameter>("Blend With Empty", 0.0, 0.0, 1.0)),
  flip(std::make_shared<Parameter>("Flip", 0.0, 0.0, 1.0)),
  alpha(std::make_shared<Parameter>("Alpha", 1.0, 0.0, 1.0)),
  alphaOscillator(std::make_shared<WaveformOscillator>(alpha)),
  ShaderSettings(shaderId, j) {
    parameters = { mode, blendWithEmpty, flip, alpha };
    oscillators = { alphaOscillator };
    load(j);
    registerParameters();
  };
};

struct BlendShader: Shader {
  BlendSettings *settings;
  BlendShader(BlendSettings *settings) : settings(settings), Shader(settings) {};
  ofShader shader;
  std::vector<std::string> blendModeNames = {
      "Multiply",
      "Screen",
      "Darken",
      "Lighten",
      "Difference",
      "Exclusion",
      "Overlay",
      "Hard Light",
      "Soft Light",
      "Color Dodge",
      "Linear Dodge",
      "Burn",
      "Linear Burn"
  };  
  void setup() override {
    shader.load("shaders/Blend");
  }

  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override {
    canvas->begin();
    shader.begin();
    shader.setUniform1i("mode", settings->mode->intValue);
    shader.setUniform1i("blendWithEmpty", settings->blendWithEmpty->intValue);
    shader.setUniform1f("time", ofGetElapsedTimef());
    shader.setUniform1f("alpha", settings->alpha->value);
    shader.setUniform2f("dimensions", frame->getWidth(), frame->getHeight());
    
    if (hasInputAtSlot(InputSlotTwo)) {
      std::shared_ptr<ofFbo> tex2 = inputAtSlot(InputSlotTwo)->frame();
      ofTexture mainTexture = settings->flip->boolValue ? tex2->getTexture() : frame->getTexture();
      ofTexture secondaryTexture = settings->flip->boolValue ? frame->getTexture() : tex2->getTexture();
      shader.setUniformTexture("tex", mainTexture, 4);
      shader.setUniformTexture("tex2", secondaryTexture, 5);
    } else {
      shader.setUniformTexture("tex", frame->getTexture(), 4);
      shader.end();
      frame->draw(0, 0);
      shader.end();
      canvas->end();
      return;
    }
    
    frame->draw(0, 0);
    shader.end();
    canvas->end();
  }

  void clear() override {
    
  }

  int inputCount() override {
    return 2;
  }

  ShaderType type() override {
    return ShaderTypeBlend;
  }

  void drawSettings() override {
    CommonViews::H3Title("Blend");
    drawSelector();
  }
  
  void drawSelector() {
    // Draw a combo selector
    ImGui::Text("Blend Mode");
    ImGui::SameLine();
    ImGui::PushItemWidth(200);
    // Convert the strings to C strings
    std::vector<const char *> blendModeNamesC;
    for (auto &name : blendModeNames) {
      blendModeNamesC.push_back(name.c_str());
    }
    ImGui::Combo("##BlendMode", &settings->mode->intValue, blendModeNamesC.data(), (int) blendModeNames.size());
    CommonViews::ShaderCheckbox(settings->blendWithEmpty);
    CommonViews::ShaderCheckbox(settings->flip);
    CommonViews::ShaderParameter(settings->alpha, settings->alphaOscillator);
  }
};

#endif /* BlendShader_hpp */
