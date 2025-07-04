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
#include "WaveformOscillator.hpp"

struct FujiSettings: public ShaderSettings {
	public:
  std::shared_ptr<Parameter> speed;

  std::shared_ptr<Parameter> cloud1Y;
  std::shared_ptr<Parameter> cloud1X;

  std::shared_ptr<Parameter> cloud2Y;
  std::shared_ptr<Parameter> cloud2X;

  std::shared_ptr<Parameter> sunSize;
  std::shared_ptr<Parameter> snowLevel;
  std::shared_ptr<Parameter> gridDensity;
  std::shared_ptr<Parameter> fujiSteepness;
  std::shared_ptr<Parameter> gridAngle;
  std::shared_ptr<Parameter> gridSquareSize;

  std::shared_ptr<Oscillator> cloud1XOscillator;
  std::shared_ptr<Oscillator> cloud1YOscillator;

  std::shared_ptr<Oscillator> cloud2XOscillator;
  std::shared_ptr<Oscillator> cloud2YOscillator;
  
  std::shared_ptr<Oscillator> speedOscillator;

  std::shared_ptr<Oscillator> sunSizeOscillator;
  std::shared_ptr<Oscillator> snowLevelOscillator;
  std::shared_ptr<Oscillator> gridDensityOscillator;
  std::shared_ptr<Oscillator> fujiSteepnessOscillator;
  std::shared_ptr<Oscillator> gridAngleOscillator;
  std::shared_ptr<Oscillator> gridSquareSizeOscillator;

  FujiSettings(std::string shaderId, json j, std::string name) :
  cloud1Y(std::make_shared<Parameter>("cloud1Y", 0.0,  -0.5, 0.5)),
  cloud1X(std::make_shared<Parameter>("cloud1X", 0.0,  0.0, 1.0)),
  cloud2Y(std::make_shared<Parameter>("cloud2Y", 0.0,  -0.5, 0.5)),
  cloud2X(std::make_shared<Parameter>("cloud2X", 0.0,  0.0, 1.0)),
  speed(std::make_shared<Parameter>("speed", 1.0, 0.0, 2.0)),
  sunSize(std::make_shared<Parameter>("Sun Size", 0.3, 0.01, 1.0)),
  snowLevel(std::make_shared<Parameter>("Snow Level", 0.2, -0.5, 1.0)),
  gridDensity(std::make_shared<Parameter>("Grid Density", 0.01, 0.001, 0.1)),
  fujiSteepness(std::make_shared<Parameter>("Fuji Steepness", 0.5, 0.1, 1.5)),
  gridAngle(std::make_shared<Parameter>("Grid Angle", 1.0, 0.0, 2.0)),
  gridSquareSize(std::make_shared<Parameter>("Grid Square Size", 1.0, 0.1, 10.0)),
  cloud1XOscillator(std::make_shared<WaveformOscillator>(cloud1X)),
  cloud1YOscillator(std::make_shared<WaveformOscillator>(cloud1Y)),
  cloud2XOscillator(std::make_shared<WaveformOscillator>(cloud2X)),
  cloud2YOscillator(std::make_shared<WaveformOscillator>(cloud2Y)),
  speedOscillator(std::make_shared<WaveformOscillator>(speed)),
  sunSizeOscillator(std::make_shared<WaveformOscillator>(sunSize)),
  snowLevelOscillator(std::make_shared<WaveformOscillator>(snowLevel)),
  gridDensityOscillator(std::make_shared<WaveformOscillator>(gridDensity)),
  fujiSteepnessOscillator(std::make_shared<WaveformOscillator>(fujiSteepness)),
  gridAngleOscillator(std::make_shared<WaveformOscillator>(gridAngle)),
  gridSquareSizeOscillator(std::make_shared<WaveformOscillator>(gridSquareSize)),
  ShaderSettings(shaderId, j, name) {
    parameters = {cloud1Y, cloud1X, cloud2Y, cloud2X, speed, sunSize, snowLevel, gridDensity, fujiSteepness, gridAngle, gridSquareSize};
    oscillators = {cloud1XOscillator, cloud1YOscillator, cloud2XOscillator, cloud2YOscillator, speedOscillator, sunSizeOscillator, snowLevelOscillator, gridDensityOscillator, fujiSteepnessOscillator, gridAngleOscillator, gridSquareSizeOscillator};
    load(j);
  registerParameters();
  };
};


class FujiShader: public Shader {
public:

  private:


public:
  FujiShader(FujiSettings *settings) :
  settings(settings),
  Shader(settings) {};

  FujiSettings *settings;

  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override {
    canvas->begin();
    shader.begin();
    shader.setUniform2f("dimensions", frame->getWidth(), frame->getHeight());
    shader.setUniform1f("time", TimeService::getService()->timeParam->value * settings->speed->value);
    shader.setUniform2f("cloud1", settings->cloud1X->value, settings->cloud1Y->value);
    shader.setUniform2f("cloud2", settings->cloud2X->value, settings->cloud2Y->value);
    shader.setUniform1f("sunSize", settings->sunSize->value);
    shader.setUniform1f("snowLevel", settings->snowLevel->value);
    shader.setUniform1f("gridDensity", settings->gridDensity->value);
    shader.setUniform1f("fujiSteepness", settings->fujiSteepness->value);
    shader.setUniform1f("gridAngle", settings->gridAngle->value);
    shader.setUniform1f("gridSquareSize", settings->gridSquareSize->value);
    frame->draw(0, 0);
    shader.end();
    canvas->end();
  };

  void setup() override {
    shader.load("shaders/fuji");
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
    CommonViews::ShaderParameter(settings->sunSize, settings->sunSizeOscillator);
    CommonViews::ShaderParameter(settings->snowLevel, settings->snowLevelOscillator);
    CommonViews::ShaderParameter(settings->gridDensity, settings->gridDensityOscillator);
    CommonViews::ShaderParameter(settings->fujiSteepness, settings->fujiSteepnessOscillator);
    CommonViews::ShaderParameter(settings->gridAngle, settings->gridAngleOscillator);
    CommonViews::ShaderParameter(settings->gridSquareSize, settings->gridSquareSizeOscillator);
  }
};

#endif
