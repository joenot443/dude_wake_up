//
//  MirrorShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef MirrorShader_hpp
#define MirrorShader_hpp

#include "ofMain.h"

#include "ShaderConfigSelectionView.hpp"
#include "Shader.hpp"
#include "WaveformOscillator.hpp"
#include "ofxImGui.h"
#include <stdio.h>

// Mirror

struct MirrorSettings : public ShaderSettings {
public:
  std::string shaderId;
  
  
  std::shared_ptr<Parameter> xOffset;
  std::shared_ptr<Oscillator> xOffsetOscillator;
  
  std::shared_ptr<Parameter> yOffset;
  std::shared_ptr<Oscillator> yOffsetOscillator;
  
  std::shared_ptr<Parameter> mirrorTypeSelection;
  
  MirrorSettings(std::string shaderId, json j, std::string name)
  : shaderId(shaderId),
  // Both - X - Y
  mirrorTypeSelection(std::make_shared<Parameter>("Mirror Type", 0.0, 0.0, 2.0, ParameterType_Int)),
  xOffset(
          std::make_shared<Parameter>("Offset X", 0.0, 0.0, 1.0)),
  yOffset(
          std::make_shared<Parameter>("Offset Y", 0.0, 0.0, 1.0)),
  xOffsetOscillator(std::make_shared<WaveformOscillator>(xOffset)),
  yOffsetOscillator(std::make_shared<WaveformOscillator>(yOffset)),
  ShaderSettings(shaderId, j, name) {
    audioReactiveParameter = xOffset;
    parameters = {mirrorTypeSelection, xOffset, yOffset};
    oscillators = {xOffsetOscillator, yOffsetOscillator};
    load(j);
    registerParameters();
  }
};

class MirrorShader : public Shader {
public:
  

  MirrorSettings *settings;
  MirrorShader(MirrorSettings *settings)
  : settings(settings), Shader(settings){};
  
  void setup() override {
#ifdef DEBUG
    shader.load("shaders/mirror");
#endif
#ifdef RELEASE
    shader.load("shaders/mirror");
#endif
    
  }
  
  int inputCount() override {
    return 1;
  }
  ShaderType type() override { return ShaderTypeMirror; }
  
  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override {
    canvas->begin();
    shader.begin();
    shader.setUniformTexture("tex", frame->getTexture(), 4);
    shader.setUniform2f("dimensions", frame->getWidth(), frame->getHeight());
    
    // X/Y Locked
    if (settings->mirrorTypeSelection->intParamValue() == 0) {
      shader.setUniform1i("mirrorXEnabled", 1);
      shader.setUniform1i("mirrorYEnabled", 1);
      shader.setUniform2f("offset", settings->xOffset->value,
                          settings->xOffset->value);
      // X/Y Separate
    } else if (settings->mirrorTypeSelection->intParamValue() == 1) {
      shader.setUniform1i("mirrorXEnabled", 1);
      shader.setUniform1i("mirrorYEnabled", 1);
      shader.setUniform2f("offset", settings->xOffset->value,
                          settings->yOffset->value);
      // X Only
    } else if (settings->mirrorTypeSelection->intParamValue() == 2) {
      shader.setUniform1i("mirrorXEnabled", 1);
      shader.setUniform1i("mirrorYEnabled", 0);
      shader.setUniform2f("offset", settings->xOffset->value,
                          0.);
      // Y Only
    } else if (settings->mirrorTypeSelection->intParamValue() == 3) {
      shader.setUniform1i("mirrorYEnabled", 1);
      shader.setUniform1i("mirrorXEnabled", 0);
      shader.setUniform2f("offset", 0.,
                          settings->yOffset->value);
    }
    
    frame->draw(0, 0);
    shader.end();
    canvas->end();
  }
  
  void drawSettings() override {
    
    CommonViews::ShaderOption(settings->mirrorTypeSelection, {"X / Y Locked", "X / Y Separate", "X Only", "Y Only"});
    
    // X/Y Locked
    if (settings->mirrorTypeSelection->intParamValue() == 0) {
      CommonViews::ShaderParameter(settings->xOffset, settings->xOffsetOscillator);
      // X/Y Separate
    } else if (settings->mirrorTypeSelection->intParamValue() == 1) {
      CommonViews::ShaderParameter(settings->xOffset, settings->xOffsetOscillator);
      CommonViews::ShaderParameter(settings->yOffset, settings->yOffsetOscillator);
      // X Only
    } else if (settings->mirrorTypeSelection->intParamValue() == 2) {
      CommonViews::ShaderParameter(settings->xOffset, settings->xOffsetOscillator);
      // Y Only
    } else if (settings->mirrorTypeSelection->intParamValue() == 2) {
      CommonViews::ShaderParameter(settings->yOffset, settings->yOffsetOscillator);
      
    }
  }
};

#endif /* EmptyShader_hpp */
