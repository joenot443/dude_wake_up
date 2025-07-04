//
//  WarpShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef WarpShader_hpp
#define WarpShader_hpp

#include "ofMain.h"
#include "ShaderSettings.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "WaveformOscillator.hpp"
#include "Parameter.hpp"
#include "Shader.hpp"
#include <stdio.h>

struct WarpSettings: public ShaderSettings {
public:
  std::shared_ptr<Parameter> amount;
  std::shared_ptr<Parameter> color;
  std::shared_ptr<WaveformOscillator> amountOscillator;
  std::shared_ptr<Parameter> noiseScale;
  std::shared_ptr<Parameter> warpSpeed;
  std::shared_ptr<Parameter> colorOffset;
  std::shared_ptr<Parameter> complexity;
  std::shared_ptr<Parameter> directionX;
  std::shared_ptr<Parameter> directionY;
  std::shared_ptr<Parameter> colormapType;
  std::shared_ptr<Parameter> edgeSoftness;
  std::shared_ptr<WaveformOscillator> noiseScaleOscillator;
  std::shared_ptr<WaveformOscillator> warpSpeedOscillator;
  std::shared_ptr<WaveformOscillator> colorOffsetOscillator;
  std::shared_ptr<WaveformOscillator> complexityOscillator;
  std::shared_ptr<WaveformOscillator> directionXOscillator;
  std::shared_ptr<WaveformOscillator> directionYOscillator;
  std::shared_ptr<WaveformOscillator> edgeSoftnessOscillator;
  
  WarpSettings(std::string shaderId, json j, std::string name) :
  amount(std::make_shared<Parameter>("Amount", 1.0, 0.0, 2.0)),
  amountOscillator(std::make_shared<WaveformOscillator>(amount)),
  noiseScale(std::make_shared<Parameter>("Noise Scale", 1.0, 0.1, 10.0)),
  noiseScaleOscillator(std::make_shared<WaveformOscillator>(noiseScale)),
  warpSpeed(std::make_shared<Parameter>("Warp Speed", 1.0, 0.0, 5.0)),
  warpSpeedOscillator(std::make_shared<WaveformOscillator>(warpSpeed)),
  colorOffset(std::make_shared<Parameter>("Color Offset", 0.0, -1.0, 1.0)),
  colorOffsetOscillator(std::make_shared<WaveformOscillator>(colorOffset)),
  complexity(std::make_shared<Parameter>("Complexity", 3.0, 1.0, 6.0)),
  complexityOscillator(std::make_shared<WaveformOscillator>(complexity)),
  directionX(std::make_shared<Parameter>("Direction X", 0.0, -1.0, 1.0)),
  directionXOscillator(std::make_shared<WaveformOscillator>(directionX)),
  directionY(std::make_shared<Parameter>("Direction Y", 0.0, -1.0, 1.0)),
  directionYOscillator(std::make_shared<WaveformOscillator>(directionY)),
  colormapType(std::make_shared<Parameter>("Colormap Type", 0, 0, 3, ParameterType_Int)),
  edgeSoftness(std::make_shared<Parameter>("Edge Softness", 1.0, 0.1, 5.0)),
  edgeSoftnessOscillator(std::make_shared<WaveformOscillator>(edgeSoftness)),
  ShaderSettings(shaderId, j, name) {
    parameters = { amount, noiseScale, warpSpeed, colorOffset, complexity, directionX, directionY, colormapType, edgeSoftness };
    oscillators = { amountOscillator, noiseScaleOscillator, warpSpeedOscillator, colorOffsetOscillator, complexityOscillator, directionXOscillator, directionYOscillator, edgeSoftnessOscillator };
    load(j);
    registerParameters();
  };
};

class WarpShader: public Shader {
public:
  WarpSettings *settings;
  WarpShader(WarpSettings *settings) : settings(settings), Shader(settings) {};

  void setup() override {
    shader.load("shaders/Warp");
  }
  
  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override {
    canvas->begin();
    shader.begin();
    shader.setUniformTexture("tex", frame->getTexture(), 4);
    shader.setUniform1f("amount", settings->amount->value);
    shader.setUniform1f("time", TimeService::getService()->timeParam->value);
    shader.setUniform2f("dimensions", frame->getWidth(), frame->getHeight());
    shader.setUniform1f("noiseScale", settings->noiseScale->value);
    shader.setUniform1f("warpSpeed", settings->warpSpeed->value);
    shader.setUniform1f("colorOffset", settings->colorOffset->value);
    shader.setUniform1f("complexity", settings->complexity->value);
    shader.setUniform2f("direction", settings->directionX->value, settings->directionY->value);
    shader.setUniform1i("colormapType", settings->colormapType->intValue);
    shader.setUniform1f("edgeSoftness", settings->edgeSoftness->value);
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
    return ShaderTypeWarp;
  }
  
  void drawSettings() override {
    CommonViews::ShaderParameter(settings->amount, settings->amountOscillator);
    CommonViews::ShaderParameter(settings->noiseScale, settings->noiseScaleOscillator);
    CommonViews::ShaderParameter(settings->warpSpeed, settings->warpSpeedOscillator);
    CommonViews::ShaderParameter(settings->colorOffset, settings->colorOffsetOscillator);
    CommonViews::ShaderIntParameter(settings->complexity);
    CommonViews::ShaderParameter(settings->directionX, settings->directionXOscillator);
    CommonViews::ShaderParameter(settings->directionY, settings->directionYOscillator);
    CommonViews::ShaderOption(settings->colormapType, {"Default", "Rainbow", "Gray", "Fire"});
    CommonViews::ShaderParameter(settings->edgeSoftness, settings->edgeSoftnessOscillator);
  }
};

#endif
