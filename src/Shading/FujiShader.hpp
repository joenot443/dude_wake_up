//
//  FujiShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 11/22/22.
//

#ifndef FujiShader_h
#define FujiShader_h

#include "ShaderConfigSelectionView.hpp"
#include "Shader.hpp"
#include "CommonViews.hpp"
 
#include "ShaderSettings.hpp"
#include "ofxImGui.h"

struct FujiSettings: public ShaderSettings {
	public:
  std::shared_ptr<Parameter> enabled;
  
  std::shared_ptr<Parameter> speed;

  std::shared_ptr<Parameter> cloud1Y;
  std::shared_ptr<Parameter> cloud1X;

  std::shared_ptr<Parameter> cloud2Y;
  std::shared_ptr<Parameter> cloud2X;

  std::shared_ptr<Oscillator> cloud1XOscillator;
  std::shared_ptr<Oscillator> cloud1YOscillator;

  std::shared_ptr<Oscillator> cloud2XOscillator;
  std::shared_ptr<Oscillator> cloud2YOscillator;
  
  std::shared_ptr<Oscillator> speedOscillator;

  FujiSettings(std::string shaderId, json j, std::string name) :
  enabled(std::make_shared<Parameter>("enabled", 0.0,  1.0, 0.0)),
  cloud1Y(std::make_shared<Parameter>("cloud1Y", 0.0,  -0.5, 0.5)),
  cloud1X(std::make_shared<Parameter>("cloud1X", 0.0,  0.0, 1.0)),
  cloud2Y(std::make_shared<Parameter>("cloud2Y", 0.0,  -0.5, 0.5)),
  cloud2X(std::make_shared<Parameter>("cloud2X", 0.0,  0.0, 1.0)),
  speed(std::make_shared<Parameter>("speed", 1.0, 0.0, 2.0)),
  cloud1XOscillator(std::make_shared<WaveformOscillator>(cloud1X)),
  cloud1YOscillator(std::make_shared<WaveformOscillator>(cloud1Y)),
  cloud2XOscillator(std::make_shared<WaveformOscillator>(cloud2X)),
  cloud2YOscillator(std::make_shared<WaveformOscillator>(cloud2Y)),
  speedOscillator(std::make_shared<WaveformOscillator>(speed)),
  ShaderSettings(shaderId, j, name) {
    parameters = {enabled, cloud1Y, cloud1X, cloud2Y, cloud2X};
    oscillators = {cloud1XOscillator, cloud1YOscillator, cloud2XOscillator, cloud2YOscillator};
    load(j);
  registerParameters();
  };
};


class FujiShader: public Shader {
public:

  private:
  ofShader shader;

public:
  FujiShader(FujiSettings *settings) :
  settings(settings),
  Shader(settings) {};

  FujiSettings *settings;

  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override {
    canvas->begin();
    shader.begin();
    shader.setUniform2f("dimensions", frame->getWidth(), frame->getHeight());
    shader.setUniform1f("time", ofGetElapsedTimef() * settings->speed->value);
    shader.setUniform2f("cloud1", settings->cloud1X->value, settings->cloud1Y->value);
    shader.setUniform2f("cloud2", settings->cloud2X->value, settings->cloud2Y->value);
    frame->draw(0, 0);
    shader.end();
    canvas->end();
  };

  void setup() override {
    #ifdef TESTING
shader.load("shaders/fuji");
#endif
#ifdef RELEASE
shader.load("shaders/fuji");
#endif
  };
  
  int inputCount() override {
    return 0;
  }

  void drawSettings() override {
    
    CommonViews::ShaderParameter(settings->cloud1X, settings->cloud1XOscillator);
    CommonViews::ShaderParameter(settings->cloud1Y, settings->cloud1YOscillator);
    CommonViews::ShaderParameter(settings->cloud2X, settings->cloud2XOscillator);
    CommonViews::ShaderParameter(settings->cloud2Y, settings->cloud2YOscillator);
    CommonViews::ShaderParameter(settings->speed, settings->speedOscillator);
  }
};

#endif /* FujiShader_h */
