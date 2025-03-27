//
//  SpiralWhirlpoolShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef SpiralWhirlpoolShader_hpp
#define SpiralWhirlpoolShader_hpp

#include "ofMain.h"
#include "ShaderSettings.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "WaveformOscillator.hpp"
#include "Parameter.hpp"
#include "Shader.hpp"
#include <stdio.h>

struct SpiralWhirlpoolSettings: public ShaderSettings {
  std::shared_ptr<Parameter> warpSize;
  std::shared_ptr<WaveformOscillator> warpSizeOscillator;
  
  std::shared_ptr<Parameter> objectSize;
  std::shared_ptr<WaveformOscillator> objectSizeOscillator;
  
  std::shared_ptr<Parameter> cameraY;
  std::shared_ptr<WaveformOscillator> cameraYOscillator;
  
  std::shared_ptr<Parameter> cameraZ;
  std::shared_ptr<WaveformOscillator> cameraZOscillator;
  
  std::shared_ptr<Parameter> colorGradient;
  std::shared_ptr<WaveformOscillator> colorGradientOscillator;
  
  std::shared_ptr<Parameter> stepSize;
  std::shared_ptr<WaveformOscillator> stepSizeOscillator;

  SpiralWhirlpoolSettings(std::string shaderId, json j) :
    warpSize(std::make_shared<Parameter>("Warp Size", 40.0, 10.0, 100.0)),
    warpSizeOscillator(std::make_shared<WaveformOscillator>(warpSize)),
    objectSize(std::make_shared<Parameter>("Object Size", 0.4, 0.1, 1.0)),
    objectSizeOscillator(std::make_shared<WaveformOscillator>(objectSize)),
    cameraY(std::make_shared<Parameter>("Camera Y", 0.415, 0.0, 1.0)),
    cameraYOscillator(std::make_shared<WaveformOscillator>(cameraY)),
    cameraZ(std::make_shared<Parameter>("Camera Z", 0.444, 0.0, 1.0)),
    cameraZOscillator(std::make_shared<WaveformOscillator>(cameraZ)),
    colorGradient(std::make_shared<Parameter>("Color Gradient", 5.0, 1.0, 10.0)),
    colorGradientOscillator(std::make_shared<WaveformOscillator>(colorGradient)),
    stepSize(std::make_shared<Parameter>("Step Size", 0.7, 0.1, 2.0)),
    stepSizeOscillator(std::make_shared<WaveformOscillator>(stepSize)),
    ShaderSettings(shaderId, j, "SpiralWhirlpool") {
      parameters = { warpSize, objectSize, cameraY, cameraZ, colorGradient, stepSize };
      oscillators = { warpSizeOscillator, objectSizeOscillator, cameraYOscillator, 
                     cameraZOscillator, colorGradientOscillator, stepSizeOscillator };
      load(j);
      registerParameters();
  };
};

struct SpiralWhirlpoolShader: Shader {
  SpiralWhirlpoolSettings *settings;
  SpiralWhirlpoolShader(SpiralWhirlpoolSettings *settings) : settings(settings), Shader(settings) {};
  
  void setup() override {
    shader.load("shaders/SpiralWhirlpool");
  }

  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override {
    canvas->begin();
    shader.begin();
    shader.setUniformTexture("tex", frame->getTexture(), 4);
    shader.setUniform1f("warpSize", settings->warpSize->value);
    shader.setUniform1f("objectSize", settings->objectSize->value);
    shader.setUniform1f("cameraY", settings->cameraY->value);
    shader.setUniform1f("cameraZ", settings->cameraZ->value);
    shader.setUniform1f("colorGradient", settings->colorGradient->value);
    shader.setUniform1f("stepSize", settings->stepSize->value);
    shader.setUniform1f("time", ofGetElapsedTimef());
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
    return ShaderTypeSpiralWhirlpool;
  }

  void drawSettings() override {
    CommonViews::H3Title("SpiralWhirlpool");
    
    CommonViews::ShaderParameter(settings->warpSize, settings->warpSizeOscillator);
    CommonViews::ShaderParameter(settings->objectSize, settings->objectSizeOscillator);
    CommonViews::ShaderParameter(settings->cameraY, settings->cameraYOscillator);
    CommonViews::ShaderParameter(settings->cameraZ, settings->cameraZOscillator);
    CommonViews::ShaderParameter(settings->colorGradient, settings->colorGradientOscillator);
    CommonViews::ShaderParameter(settings->stepSize, settings->stepSizeOscillator);
  }
};

#endif /* SpiralWhirlpoolShader_hpp */
