//
//  DiscoSunShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef DiscoSunShader_hpp
#define DiscoSunShader_hpp

#include "ofMain.h"
#include "ShaderSettings.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "TimeService.hpp"
#include "WaveformOscillator.hpp"
#include "Parameter.hpp"
#include "Shader.hpp"
#include <stdio.h>

struct DiscoSunSettings: public ShaderSettings {
  std::shared_ptr<Parameter> speed;
  std::shared_ptr<WaveformOscillator> speedOscillator;

  std::shared_ptr<Parameter> twist;
  std::shared_ptr<WaveformOscillator> twistOscillator;

  std::shared_ptr<Parameter> shimmer;
  std::shared_ptr<WaveformOscillator> shimmerOscillator;

  std::shared_ptr<Parameter> brightness;
  std::shared_ptr<WaveformOscillator> brightnessOscillator;

  DiscoSunSettings(std::string shaderId, json j) :
  speed(std::make_shared<Parameter>("Speed", 1.0, 0.0, 4.0)),
  speedOscillator(std::make_shared<WaveformOscillator>(speed)),
  twist(std::make_shared<Parameter>("Twist", 1.0, 0.0, 5.0)),
  twistOscillator(std::make_shared<WaveformOscillator>(twist)),
  shimmer(std::make_shared<Parameter>("Shimmer", 1.0, 0.0, 2.0)),
  shimmerOscillator(std::make_shared<WaveformOscillator>(shimmer)),
  brightness(std::make_shared<Parameter>("Brightness", 1.0, 0.1, 3.0)),
  brightnessOscillator(std::make_shared<WaveformOscillator>(brightness)),
  ShaderSettings(shaderId, j, "DiscoSun") {
    parameters = { speed, twist, shimmer, brightness };
    oscillators = { speedOscillator, twistOscillator, shimmerOscillator, brightnessOscillator };
    load(j);
    registerParameters();
  };
};

struct DiscoSunShader: Shader {
  DiscoSunSettings *settings;
  DiscoSunShader(DiscoSunSettings *settings) : settings(settings), Shader(settings) {};
  
  void setup() override {
    shader.load("shaders/DiscoSun");
  }

  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override {
    canvas->begin();
    shader.begin();
    shader.setUniformTexture("tex", frame->getTexture(), 4);
    shader.setUniform1f("speed", settings->speed->value);
    shader.setUniform1f("twist", settings->twist->value);
    shader.setUniform1f("shimmer", settings->shimmer->value);
    shader.setUniform1f("brightness", settings->brightness->value);
    shader.setUniform1f("time", TimeService::getService()->timeParam->value);
    shader.setUniform2f("dimensions", frame->getWidth(), frame->getHeight());
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
    return ShaderTypeDiscoSun;
  }

  void drawSettings() override {
    CommonViews::H3Title("DiscoSun");
    CommonViews::ShaderParameter(settings->speed, settings->speedOscillator);
    CommonViews::ShaderParameter(settings->twist, settings->twistOscillator);
    CommonViews::ShaderParameter(settings->shimmer, settings->shimmerOscillator);
    CommonViews::ShaderParameter(settings->brightness, settings->brightnessOscillator);
  }
};

#endif /* DiscoSunShader_hpp */
