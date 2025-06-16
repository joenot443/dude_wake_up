//
//  MelterShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 12/2/22.
//

#ifndef MelterShader_h
#define MelterShader_h

#include "ofMain.h"

#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "Oscillator.hpp"
#include "ShaderConfigSelectionView.hpp"
#include "Shader.hpp"
#include <stdio.h>

struct MeltSettings : public ShaderSettings {
	public:
  std::string shaderId;
  std::shared_ptr<Parameter> spread;
  std::shared_ptr<WaveformOscillator> spreadOscillator;

  std::shared_ptr<Parameter> offset;
  std::shared_ptr<WaveformOscillator> offsetOscillator;

  std::shared_ptr<Parameter> cycles;
  std::shared_ptr<WaveformOscillator> cyclesOscillator;

  std::shared_ptr<Parameter> speed;
  std::shared_ptr<WaveformOscillator> speedOscillator;
  
  std::shared_ptr<Parameter> smoothness;
  std::shared_ptr<WaveformOscillator> smoothnessOscillator;

  MeltSettings(std::string shaderId, json j, std::string name) : shaderId(shaderId),
  spread(std::make_shared<Parameter>("Spread", 1.0, 0.0, 2.0)),
  spreadOscillator(std::make_shared<WaveformOscillator>(spread)),
  offset(std::make_shared<Parameter>("Offset", 0.0, -1.0, 1.0)),
  offsetOscillator(std::make_shared<WaveformOscillator>(offset)),
  cycles(std::make_shared<Parameter>("Cycles", 1.0, 0.0, 5.0)),
  cyclesOscillator(std::make_shared<WaveformOscillator>(cycles)),
  speed(std::make_shared<Parameter>("Speed", 1.0, 0.0, 5.0)),
  speedOscillator(std::make_shared<WaveformOscillator>(speed)),
  smoothness(std::make_shared<Parameter>("Smoothness", 0.6, 0.0, 1.0)),
  smoothnessOscillator(std::make_shared<WaveformOscillator>(smoothness)),
  ShaderSettings(shaderId, j, name) {
    oscillators = {spreadOscillator, speedOscillator, smoothnessOscillator, cyclesOscillator, offsetOscillator};
    parameters = {spread, speed, smoothness, cycles, offset };
    audioReactiveParameter = spread;
    load(j);
    registerParameters();
  };
};

class MeltShader : public Shader {
public:

  private:


  public:
  MeltSettings *settings;

  MeltShader(MeltSettings *settings) : settings(settings),
  Shader(settings) {
    shader.load("shaders/Melt");
  }
  
  int inputCount() override {
    return 0;
  }
  
  std::string fileName() override {
    return "Melt";
  }

  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override {
    canvas->begin();
    shader.begin();
    shader.setUniformTexture("tex", frame->getTexture(), 4);
    shader.setUniform1f("time", ofGetElapsedTimef());
    shader.setUniform1f("spread", settings->spread->value);
    shader.setUniform1f("cycles", settings->cycles->value); 
    shader.setUniform1f("speed", settings->speed->value);
    shader.setUniform1f("smoothness", 1.0f - settings->smoothness->value);
    shader.setUniform1f("offset", settings->offset->value);
    shader.setUniform2f("dimensions", frame->getWidth(), frame->getHeight());
    frame->draw(0, 0);
    shader.end();
    canvas->end();
  }

  void drawSettings() override {    
    CommonViews::ShaderParameter(settings-> spread, settings->spreadOscillator);
    CommonViews::ShaderParameter(settings->speed, settings->speedOscillator);
    CommonViews::ShaderParameter(settings->smoothness, settings->smoothnessOscillator);
    CommonViews::ShaderParameter(settings->cycles, settings->cyclesOscillator);
    CommonViews::ShaderParameter(settings->offset, settings->offsetOscillator);
  }
};
#endif
