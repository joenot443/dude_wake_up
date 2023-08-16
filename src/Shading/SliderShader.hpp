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
  std::shared_ptr<Parameter> speed;
  std::shared_ptr<Parameter> vertical;

  std::shared_ptr<Oscillator> speedOscillator;

  SliderSettings(std::string shaderId, json j)
      : speed(std::make_shared<Parameter>("Speed", 0.5, 0.01, 2.0)),
      vertical(std::make_shared<Parameter>("Vertical", 0., 0., 1.)),
        speedOscillator(std::make_shared<WaveformOscillator>(speed)),
        ShaderSettings(shaderId, j){
          parameters = { speed, vertical };
          oscillators = { speedOscillator };
          registerParameters();
        };
};

struct SliderShader : Shader {
  SliderSettings *settings;
  SliderShader(SliderSettings *settings)
      : settings(settings), Shader(settings){};
  ofShader shader;
  void setup() override {
#ifdef TESTING
shader.load("shaders/Slider");
#endif
#ifdef RELEASE
shader.load("shaders/Slider");
#endif
    
  }

  void shade(ofFbo *frame, ofFbo *canvas) override {
    canvas->begin();
    shader.begin();
    shader.setUniformTexture("tex", frame->getTexture(), 4);
    shader.setUniform1f("time", ofGetElapsedTimef());
    shader.setUniform1f("speed", settings->speed->value);
    shader.setUniform1i("vertical", settings->vertical->boolValue);
    frame->draw(0, 0);
    shader.end();
    canvas->end();
  }

  void clear() override {}

  ShaderType type() override { return ShaderTypeSlider; }

  void drawSettings() override {
    ShaderConfigSelectionView::draw(this);
    CommonViews::H3Title("Slider");

    CommonViews::ShaderParameter(settings->speed, settings->speedOscillator);
    CommonViews::ShaderCheckbox(settings->vertical);
  }
};

#endif /* SliderShader_hpp */
