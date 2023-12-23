//
//  CloudShader.h
//  dude_wake_up
//
//  Created by Joe Crozier on 12/2/22.
//

#ifndef CloudShader_h
#define CloudShader_h

#include "ofMain.h"
#include "WaveformOscillator.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "Oscillator.hpp"
#include "ShaderConfigSelectionView.hpp"
#include "Shader.hpp"
#include <stdio.h>

struct CloudSettings : public ShaderSettings {
	public:
  std::string shaderId;
  std::shared_ptr<Parameter> speed;

  std::shared_ptr<Oscillator> speedOscillator;

  CloudSettings(std::string shaderId, json j) :
  speed(std::make_shared<Parameter>("speed", 1.0, 0.0, 2.0)),
  speedOscillator(std::make_shared<WaveformOscillator>(speed)),
  ShaderSettings(shaderId, j) {
    parameters = {speed};
    oscillators = {speedOscillator};
    load(j);
  registerParameters();
  };
};

class CloudShader: public Shader {
  private:
  ofShader shader;

  public:
  CloudSettings *settings;

  CloudShader(CloudSettings *settings) :
  settings(settings),
  Shader(settings) {};

  void setup() override {
    #ifdef TESTING
shader.load("shaders/clouds");
#endif
#ifdef RELEASE
shader.load("shaders/clouds");
#endif
  }

  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override {
    canvas->begin();
    shader.begin();
    shader.setUniformTexture("tex", frame->getTexture(), 4);
    shader.setUniform1f("time", ofGetElapsedTimef() * settings->speed->value);
    shader.setUniform2f("dimensions", frame->getWidth(), frame->getHeight());
    frame->draw(0, 0);
    shader.end();
    canvas->end();
  }

  void drawSettings() override {
    ImGui::Text("Clouds");
    CommonViews::ShaderParameter(settings->speed, settings->speedOscillator);
  }


};

#endif /* CloudShader_h */
