//
//  MirrorShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef MirrorShader_hpp
#define MirrorShader_hpp

#include "ofMain.h"
#include "VideoSettings.hpp"
#include "ofxImGui.h"
#include "Shader.hpp"
#include <stdio.h>

// Mirror

struct MirrorSettings: public ShaderSettings  {
  std::string shaderId;
  
  std::shared_ptr<Parameter> mirrorXEnabled;
  std::shared_ptr<Parameter> mirrorYEnabled;
  
  std::shared_ptr<Parameter> xOffset;
  std::shared_ptr<Oscillator> xOffsetOscillator;
  
  std::shared_ptr<Parameter> yOffset;
  std::shared_ptr<Oscillator> yOffsetOscillator;
  
  std::shared_ptr<Parameter> lockXY;

  MirrorSettings(std::string shaderId, json j) :
  shaderId(shaderId),
  lockXY(std::make_shared<Parameter>("lockXY", shaderId, 0.0, 1.0, 1.0)),
  xOffset(std::make_shared<Parameter>("xOffset", shaderId, 0.15,  0.01, 1.0)),
  yOffset(std::make_shared<Parameter>("yOffset", shaderId, 0.15,  0.01, 1.0)),
  mirrorXEnabled(std::make_shared<Parameter>("mirrorXEnabled", shaderId, 0.0,  1.0, 0.0)),
  mirrorYEnabled(std::make_shared<Parameter>("mirrorXEnabled", shaderId, 0.0,  1.0, 0.0)),
  xOffsetOscillator(std::make_shared<Oscillator>(xOffset)),
  yOffsetOscillator(std::make_shared<Oscillator>(yOffset)),
  ShaderSettings(shaderId)
  {
    parameters = {lockXY, xOffset, yOffset, mirrorXEnabled, mirrorYEnabled};
    oscillators = {xOffsetOscillator, yOffsetOscillator};
    load(j);
  }
};

struct MirrorShader: Shader {
  ofShader shader;
  MirrorSettings *settings;
  MirrorShader(MirrorSettings *settings) : settings(settings), Shader(settings) {};
  
  void setup() override {
    shader.load("shaders/mirror");
  }
  
  ShaderType type() override {
    return ShaderTypeMirror;
  }
  
  void shade(ofFbo *frame, ofFbo *canvas) override {
    canvas->begin();
    shader.begin();
    shader.setUniformTexture("tex", frame->getTexture(), 4);
    shader.setUniform1i("mirrorXEnabled", settings->mirrorXEnabled->boolValue);
    shader.setUniform1i("mirrorYEnabled", settings->mirrorYEnabled->boolValue);
    shader.setUniform2f("dimensions", frame->getWidth(), frame->getHeight());
    
    if (settings->lockXY->boolValue) {
      shader.setUniform2f("offset", settings->xOffset->value, settings->xOffset->value);
    } else {
      shader.setUniform2f("offset", settings->xOffset->value, settings->yOffset->value);
    }
    
    frame->draw(0, 0);
    shader.end();
    canvas->end();
  }
  
  void drawSettings() override {
    CommonViews::H3Title("Mirror");
    
    // Mirror X
    ImGui::Text("Mirror X Enabled");
    ImGui::SetNextItemWidth(150.0);
    ImGui::SameLine(0, 20);
    ImGui::Checkbox("Enabled##mirror_x_enabled", &settings->mirrorXEnabled->boolValue);
    
    // Mirror Y
    ImGui::Text("Mirror Y Enabled");
    ImGui::SetNextItemWidth(150.0);
    ImGui::SameLine(0, 20);
    ImGui::Checkbox("Enabled##mirror_y_enabled", &settings->mirrorYEnabled->boolValue);
    
    // Lock XY
    ImGui::Text("Lock XY Enabled");
    ImGui::SetNextItemWidth(150.0);
    ImGui::SameLine(0, 20);
    ImGui::Checkbox("Enabled##lock_xy_enabled", &settings->lockXY->boolValue);
    
    
    if (settings->lockXY->boolValue) {
      // XY Offset
      CommonViews::Slider("X/Y Offset", "##xyoffset", settings->xOffset);
      CommonViews::ModulationSelector(settings->xOffset);
      CommonViews::MidiSelector(settings->xOffset);
    } else {
      // X Offset
      CommonViews::Slider("X Offset", "##xoffset", settings->xOffset);
      CommonViews::ModulationSelector(settings->xOffset);
      CommonViews::MidiSelector(settings->xOffset);
      
      // Y Offset
      CommonViews::Slider("Y Offset", "##yoffset", settings->yOffset);
      CommonViews::ModulationSelector(settings->yOffset);
      CommonViews::MidiSelector(settings->yOffset);
    }
  }
};

#endif /* EmptyShader_hpp */
