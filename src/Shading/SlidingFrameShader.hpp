//
//  SlidingFrameShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef SlidingFrameShader_hpp
#define SlidingFrameShader_hpp

#include "ofMain.h"
#include "ShaderSettings.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "WaveformOscillator.hpp"
#include "Parameter.hpp"
#include "Shader.hpp"
#include <stdio.h>

struct SlidingFrameSettings: public ShaderSettings {
  std::shared_ptr<Parameter> speed;
  std::shared_ptr<WaveformOscillator> speedOscillator;
  
  std::shared_ptr<Parameter> lineWidth;
  std::shared_ptr<WaveformOscillator> lineWidthOscillator;
  
  std::shared_ptr<Parameter> angle;
  std::shared_ptr<WaveformOscillator> angleOscillator;
  
  SlidingFrameSettings(std::string shaderId, json j, std::string name) :
  speed(std::make_shared<Parameter>("Speed", 100.0, 0.0, 500.0)),
  lineWidth(std::make_shared<Parameter>("Line Width", 10.0, 0.0, 50.0)),
  angle(std::make_shared<Parameter>("Angle", 0.0, 0.0, M_PI)),
  speedOscillator(std::make_shared<WaveformOscillator>(speed)),
  lineWidthOscillator(std::make_shared<WaveformOscillator>(lineWidth)),
  angleOscillator(std::make_shared<WaveformOscillator>(angle)),

  ShaderSettings(shaderId, j, name) {
    parameters = { speed, lineWidth, angle };
    oscillators = { speedOscillator, lineWidthOscillator, angleOscillator };
    load(j);
    registerParameters();
  };
};

struct SlidingFrameShader: Shader {
  SlidingFrameSettings *settings;
  SlidingFrameShader(SlidingFrameSettings *settings) : settings(settings), Shader(settings) {};
  ofShader shader;
  void setup() override {
    shader.load("shaders/SlidingFrame");
  }
  
  void shade(std::shared_ptr<ofFbo> SlidingFrame, std::shared_ptr<ofFbo> canvas) override {
    canvas->begin();
    shader.begin();
    if (hasInputAtSlot(InputSlotTwo)) {
      std::shared_ptr<ofFbo> tex2 = inputAtSlot(InputSlotTwo)->frame();
      shader.setUniformTexture("tex2", tex2->getTexture(), 8);
    } else {
      shader.setUniformTexture("tex2", SlidingFrame->getTexture(), 8);
    }
    
    shader.setUniformTexture("tex", SlidingFrame->getTexture(), 4);
    shader.setUniform1f("time", ofGetElapsedTimef());
    shader.setUniform1f("lineWidth", settings->lineWidth->value);
    shader.setUniform1f("speed", settings->speed->value);
    shader.setUniform1f("angle", settings->angle->value);
    shader.setUniform2f("dimensions", SlidingFrame->getWidth(), SlidingFrame->getHeight());
    SlidingFrame->draw(0, 0);
    shader.end();
    canvas->end();
  }
  
  void clear() override {
    
  }
  
  int inputCount() override {
    return 2;
  }
ShaderType type() override {
    return ShaderTypeSlidingFrame;
  }
  
  void drawSettings() override {
    CommonViews::H3Title("SlidingFrame");
    
    CommonViews::ShaderParameter(settings->speed, settings->speedOscillator);
    CommonViews::ShaderParameter(settings->angle, settings->angleOscillator);
    CommonViews::ShaderParameter(settings->lineWidth, settings->lineWidthOscillator);
  }
};

#endif /* SlidingFrameShader_hpp */
