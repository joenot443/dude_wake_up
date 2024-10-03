//
//  RainbowRotatorShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef RainbowRotatorShader_hpp
#define RainbowRotatorShader_hpp

#include "ofMain.h"
#include "ShaderSettings.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "ShaderConfigSelectionView.hpp"
#include "Shader.hpp"
#include <stdio.h>

struct RainbowRotatorSettings: public ShaderSettings {
	public:
  std::shared_ptr<Parameter> frequency;
  std::shared_ptr<WaveformOscillator> frequencyOscillator;
  std::shared_ptr<Parameter> height;
  std::shared_ptr<WaveformOscillator> heightOscillator;
  
  RainbowRotatorSettings(std::string shaderId, json j, std::string name) :
  frequency(std::make_shared<Parameter>("frequency", 1.0, 0.0, 5.0)),
  frequencyOscillator(std::make_shared<WaveformOscillator>(frequency)),
  height(std::make_shared<Parameter>("height", 1.0, 0.0, 2.0)),
  heightOscillator(std::make_shared<WaveformOscillator>(height)),
  ShaderSettings(shaderId, j, name) {
    
  };
};

class RainbowRotatorShader: public Shader {
public:

  RainbowRotatorSettings *settings;
  RainbowRotatorShader(RainbowRotatorSettings *settings) : settings(settings), Shader(settings) {};

  void setup() override {
    #ifdef TESTING
shader.load("shaders/RainbowRotator");
#endif
#ifdef RELEASE
shader.load("shaders/RainbowRotator");
#endif
  }

  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override {
    canvas->begin();
    shader.begin();
    shader.setUniformTexture("tex", frame->getTexture(), 4);
    shader.setUniform1f("time", ofGetElapsedTimef());
    shader.setUniform2f("dimensions", frame->getWidth(), frame->getHeight());
    shader.setUniform1f("frequency", settings->frequency->value);
    shader.setUniform1f("height", settings->height->value);
    frame->draw(0, 0);
    shader.end();
    canvas->end();
  }

  void clear() override {
    
  }

    int inputCount() override {
    return 1;
  }
ShaderType type() override {
    return ShaderTypeRainbowRotator;
  }

  void drawSettings() override {
    CommonViews::ShaderParameter(settings->height, settings->heightOscillator);
    CommonViews::ShaderParameter(settings->frequency, settings->frequencyOscillator);
  }
};

#endif /* RainbowRotatorShader_hpp */
