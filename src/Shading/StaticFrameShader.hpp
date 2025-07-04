//
//  StaticFrameShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef StaticFrameShader_hpp
#define StaticFrameShader_hpp

#include "ofMain.h"
#include "ShaderSettings.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "WaveformOscillator.hpp"
#include "Parameter.hpp"
#include "Shader.hpp"
#include <stdio.h>

struct StaticFrameSettings: public ShaderSettings {
  std::shared_ptr<Parameter> position;
  std::shared_ptr<WaveformOscillator> positionOscillator;
  
  std::shared_ptr<Parameter> lineWidth;
  std::shared_ptr<WaveformOscillator> lineWidthOscillator;
  
  std::shared_ptr<Parameter> angle;
  std::shared_ptr<WaveformOscillator> angleOscillator;

  StaticFrameSettings(std::string shaderId, json j, std::string name) :
  position(std::make_shared<Parameter>("Position", 0.5, 0.0, 1.0)),
  lineWidth(std::make_shared<Parameter>("Line Width", 10.0, 0.0, 50.0)),
  angle(std::make_shared<Parameter>("Angle", 0.0, 0.0, M_PI)),
  positionOscillator(std::make_shared<WaveformOscillator>(position)),
  lineWidthOscillator(std::make_shared<WaveformOscillator>(lineWidth)),
  angleOscillator(std::make_shared<WaveformOscillator>(angle)),
  ShaderSettings(shaderId, j, name) {
    parameters = { position, lineWidth, angle };
    oscillators = { positionOscillator, lineWidthOscillator, angleOscillator };
    load(j);
    registerParameters();
  };
};

struct StaticFrameShader: Shader {
  StaticFrameSettings *settings;
  StaticFrameShader(StaticFrameSettings *settings) : settings(settings), Shader(settings) {};

  void setup() override {
    shader.load("shaders/StaticFrame");
  }

  void shade(std::shared_ptr<ofFbo> StaticFrame, std::shared_ptr<ofFbo> canvas) override {
    canvas->begin();
    shader.begin();
    if (hasInputAtSlot(InputSlotTwo)) {
      std::shared_ptr<ofFbo> tex2 = inputAtSlot(InputSlotTwo)->frame();
      shader.setUniformTexture("tex2", tex2->getTexture(), 8);
    } else {
      shader.setUniformTexture("tex2", StaticFrame->getTexture(), 8);
    }
    
    shader.setUniformTexture("tex", StaticFrame->getTexture(), 4);
    shader.setUniform1f("time", TimeService::getService()->timeParam->value);
    shader.setUniform1f("lineWidth", settings->lineWidth->value);
    shader.setUniform1f("dividerPosition", settings->position->value);
    shader.setUniform1f("angle", settings->angle->value);
    shader.setUniform2f("dimensions", StaticFrame->getWidth(),
                        StaticFrame->getHeight());
    StaticFrame->draw(0, 0);
    shader.end();
    canvas->end();
  }

  void clear() override {
    
  }

  int inputCount() override {
    return 2;
  }
  
ShaderType type() override {
    return ShaderTypeStaticFrame;
  }

  void drawSettings() override {
    

    CommonViews::ShaderParameter(settings->position, settings->positionOscillator);
    CommonViews::ShaderParameter(settings->lineWidth, settings->lineWidthOscillator);
    CommonViews::ShaderParameter(settings->angle, settings->angleOscillator);

  }
};

#endif
