//
//  ComicbookShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef ComicbookShader_hpp
#define ComicbookShader_hpp

#include "ofMain.h"
#include "ShaderSettings.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "TimeService.hpp"
#include "WaveformOscillator.hpp"
#include "Parameter.hpp"
#include "Shader.hpp"
#include <stdio.h>

struct ComicbookSettings: public ShaderSettings {
  std::shared_ptr<Parameter> edgeThreshold;
  std::shared_ptr<WaveformOscillator> edgeThresholdOscillator;

  std::shared_ptr<Parameter> edgeWidth;
  std::shared_ptr<WaveformOscillator> edgeWidthOscillator;

  std::shared_ptr<Parameter> edgeSoftness;
  std::shared_ptr<WaveformOscillator> edgeSoftnessOscillator;

  std::shared_ptr<Parameter> saturation;
  std::shared_ptr<WaveformOscillator> saturationOscillator;

  std::shared_ptr<Parameter> posterize;
  std::shared_ptr<WaveformOscillator> posterizeOscillator;

  std::shared_ptr<Parameter> colorBleed;
  std::shared_ptr<WaveformOscillator> colorBleedOscillator;

  ComicbookSettings(std::string shaderId, json j) :
  edgeThreshold(std::make_shared<Parameter>("Edge Threshold", 0.8, 0.0, 1.0)),
  edgeThresholdOscillator(std::make_shared<WaveformOscillator>(edgeThreshold)),
  edgeWidth(std::make_shared<Parameter>("Edge Width", 1.0, 0.5, 5.0)),
  edgeWidthOscillator(std::make_shared<WaveformOscillator>(edgeWidth)),
  edgeSoftness(std::make_shared<Parameter>("Edge Softness", 0.0, 0.0, 0.3)),
  edgeSoftnessOscillator(std::make_shared<WaveformOscillator>(edgeSoftness)),
  saturation(std::make_shared<Parameter>("Saturation", 1.0, 0.0, 1.0)),
  saturationOscillator(std::make_shared<WaveformOscillator>(saturation)),
  posterize(std::make_shared<Parameter>("Posterize", 0.0, 0.0, 1.0)),
  posterizeOscillator(std::make_shared<WaveformOscillator>(posterize)),
  colorBleed(std::make_shared<Parameter>("Color Bleed", 0.0, 0.0, 1.0)),
  colorBleedOscillator(std::make_shared<WaveformOscillator>(colorBleed)),
  ShaderSettings(shaderId, j, "Comicbook") {
    parameters = { edgeThreshold, edgeWidth, edgeSoftness, saturation, posterize, colorBleed };
    oscillators = { edgeThresholdOscillator, edgeWidthOscillator, edgeSoftnessOscillator, saturationOscillator, posterizeOscillator, colorBleedOscillator };
    load(j);
    registerParameters();
  };
};

struct ComicbookShader: Shader {
  ComicbookSettings *settings;
  ComicbookShader(ComicbookSettings *settings) : settings(settings), Shader(settings) {};
  ofShader shader;

  void setup() override {
    shader.load("shaders/Comicbook");
  }

  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override {
    canvas->begin();
    shader.begin();
    shader.setUniformTexture("tex", frame->getTexture(), 4);
    shader.setUniform1f("time", TimeService::getService()->timeParam->value);
    shader.setUniform2f("dimensions", frame->getWidth(), frame->getHeight());
    shader.setUniform1f("edgeThreshold", settings->edgeThreshold->value);
    shader.setUniform1f("edgeWidth", settings->edgeWidth->value);
    shader.setUniform1f("edgeSoftness", settings->edgeSoftness->value);
    shader.setUniform1f("saturation", settings->saturation->value);
    shader.setUniform1f("posterize", settings->posterize->value);
    shader.setUniform1f("colorBleed", settings->colorBleed->value);
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
    return ShaderTypeComicbook;
  }

  void drawSettings() override {
    CommonViews::H3Title("Comicbook");

    CommonViews::ShaderParameter(settings->edgeThreshold, settings->edgeThresholdOscillator);
    CommonViews::ShaderParameter(settings->edgeWidth, settings->edgeWidthOscillator);
    CommonViews::ShaderParameter(settings->edgeSoftness, settings->edgeSoftnessOscillator);
    CommonViews::ShaderParameter(settings->saturation, settings->saturationOscillator);
    CommonViews::ShaderParameter(settings->posterize, settings->posterizeOscillator);
    CommonViews::ShaderParameter(settings->colorBleed, settings->colorBleedOscillator);
  }
};

#endif /* ComicbookShader_hpp */
