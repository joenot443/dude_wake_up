//
//  SliderShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef SliderShader_hpp
#define SliderShader_hpp

#include "CommonViews.hpp"
#include "Oscillator.hpp"
#include "ShaderConfigSelectionView.hpp"
#include "Shader.hpp"
#include "ShaderSettings.hpp"
#include "ofMain.h"
#include "ofxImGui.h"
#include <stdio.h>

struct SliderSettings : public ShaderSettings {
	public:
  std::shared_ptr<Parameter> speed;
  std::shared_ptr<Parameter> vertical;

  std::shared_ptr<Oscillator> speedOscillator;

  SliderSettings(std::string shaderId, json j, std::string name)
      : speed(std::make_shared<Parameter>("Speed", 0.1, 0.01, 1.0)),
      vertical(std::make_shared<Parameter>("Vertical", ParameterType_Bool)),
        speedOscillator(std::make_shared<WaveformOscillator>(speed)),
        ShaderSettings(shaderId, j, name){
          parameters = { speed, vertical };
          oscillators = { speedOscillator };
          registerParameters();
        };
};

class SliderShader : public Shader {
public:

  SliderSettings *settings;
  SliderShader(SliderSettings *settings)
      : settings(settings), Shader(settings){};

  void setup() override {
shader.load("shaders/Slider");
shader.load("shaders/Slider");
    
  }

  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override {
    canvas->begin();
    shader.begin();
    // Clear the frame
    ofClear(0,0,0, 255);
    ofClear(0,0,0, 0);
    shader.setUniformTexture("tex", frame->getTexture(), 4);
    shader.setUniform1f("time", TimeService::getService()->timeParam->value);
    shader.setUniform1f("speed", settings->speed->value);
    shader.setUniform1i("vertical", settings->vertical->boolValue);
    frame->draw(0, 0);
    shader.end();
    canvas->end();
  }

  void clear() override {}

    int inputCount() override {
    return 1;
  }
ShaderType type() override { return ShaderTypeSlider; }

  void drawSettings() override {
    
    

    CommonViews::ShaderParameter(settings->speed, settings->speedOscillator);
    CommonViews::ShaderCheckbox(settings->vertical);
  }
};

#endif
