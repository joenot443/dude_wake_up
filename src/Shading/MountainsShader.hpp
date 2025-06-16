#ifndef MountainsShader_hpp
#define MountainsShader_hpp

#include "ofMain.h"
#include "ShaderSettings.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "Parameter.hpp"
#include "Shader.hpp"
#include <stdio.h>

struct MountainsSettings: public ShaderSettings {
  public:
  std::shared_ptr<Parameter> color;
  std::shared_ptr<Parameter> mountainColor;
  std::shared_ptr<Parameter> valleyColor;
  std::shared_ptr<Parameter> alpha;
  std::shared_ptr<Parameter> heightScale;
  std::shared_ptr<Parameter> yTranslate;
  std::shared_ptr<Parameter> zDistance;
  std::shared_ptr<Parameter> rotateX;
  std::shared_ptr<Parameter> rotateY;
  std::shared_ptr<Parameter> rotateZ;
  std::shared_ptr<Parameter> rotateW;
  std::shared_ptr<Parameter> fade;

  std::shared_ptr<WaveformOscillator> heightScaleOscillator;
  std::shared_ptr<WaveformOscillator> yTranslateOscillator;
  std::shared_ptr<WaveformOscillator> zDistanceOscillator;
  std::shared_ptr<WaveformOscillator> rotateXOscillator;
  std::shared_ptr<WaveformOscillator> rotateYOscillator;
  std::shared_ptr<WaveformOscillator> rotateZOscillator;
  std::shared_ptr<WaveformOscillator> rotateWOscillator;
  std::shared_ptr<WaveformOscillator> fadeOscillator;

  MountainsSettings(std::string shaderId, json j, std::string name) :
  color(std::make_shared<Parameter>("Background Color", 1.0, -1.0, 2.0)),
  mountainColor(std::make_shared<Parameter>("Mountain Color", 1.0, -1.0, 2.0)),
  valleyColor(std::make_shared<Parameter>("Valley Color", 1.0, -1.0, 2.0)),
  alpha(std::make_shared<Parameter>("Alpha", 1.0, 0.0, 1.0)),
  heightScale(std::make_shared<Parameter>("Height Scale", 0.5, 0.0, 2.0)),
  yTranslate(std::make_shared<Parameter>("Y Translate", 1.0, -2.0, 2.0)),
  zDistance(std::make_shared<Parameter>("Z Distance", 1.0, 0.0, 2.0)),
  rotateX(std::make_shared<Parameter>("Crinkle", 0.80, 0.0, 3.0)),
  rotateY(std::make_shared<Parameter>("Stretch", 0.60, 0.0, 1.0)),
  rotateZ(std::make_shared<Parameter>("Slide", -0.10, 0.0, 1.0)),
  rotateW(std::make_shared<Parameter>("Angle", 0.80, 0.0, 1.0)),
  fade(std::make_shared<Parameter>("Fade", 0.3, 0.0, 0.5)),
  heightScaleOscillator(std::make_shared<WaveformOscillator>(heightScale)),
  yTranslateOscillator(std::make_shared<WaveformOscillator>(yTranslate)),
  zDistanceOscillator(std::make_shared<WaveformOscillator>(zDistance)),
  rotateXOscillator(std::make_shared<WaveformOscillator>(rotateX)),
  rotateYOscillator(std::make_shared<WaveformOscillator>(rotateY)),
  rotateZOscillator(std::make_shared<WaveformOscillator>(rotateZ)),
  rotateWOscillator(std::make_shared<WaveformOscillator>(rotateW)),
  fadeOscillator(std::make_shared<WaveformOscillator>(fade)),
  ShaderSettings(shaderId, j, name) {
    parameters = { color, alpha, heightScale, yTranslate, rotateX, rotateY, rotateZ, rotateW, zDistance, mountainColor, valleyColor, fade };
    oscillators = { heightScaleOscillator, yTranslateOscillator, zDistanceOscillator, rotateXOscillator, rotateYOscillator, rotateZOscillator, rotateWOscillator, fadeOscillator };
    
    mountainColor->setColor({1.0, 1.0, 1.0, 1.0});
    valleyColor->setColor({1.0, 1.0, 1.0, 1.0});
    
    load(j);
    registerParameters();
  };
};

class MountainsShader: public Shader {
public:

  MountainsSettings *settings;
  MountainsShader(MountainsSettings *settings) : settings(settings), Shader(settings) {};
	
  void setup() override {
    shader.load("shaders/Mountains");
  }

  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override {
    canvas->begin();
    shader.begin();
    shader.setUniformTexture("tex", frame->getTexture(), 4);
    shader.setUniform1f("time", ofGetElapsedTimef());
    shader.setUniform2f("dimensions", frame->getWidth(), frame->getHeight());
    shader.setUniform4f("color", settings->color->color->data()[0], settings->color->color->data()[1], settings->color->color->data()[2], settings->alpha->value);
    shader.setUniform3f("mountainColor", settings->mountainColor->color->data()[0], settings->mountainColor->color->data()[1], settings->mountainColor->color->data()[2]);
    shader.setUniform3f("valleyColor", settings->valleyColor->color->data()[0], settings->valleyColor->color->data()[1], settings->valleyColor->color->data()[2]);
    shader.setUniform1f("heightScale", settings->heightScale->value);
    shader.setUniform1f("yTranslate", settings->yTranslate->value);
    shader.setUniform1f("zDistance", settings->zDistance->value);
    shader.setUniform1f("rotateX", settings->rotateX->value);
    shader.setUniform1f("rotateY", settings->rotateY->value);
    shader.setUniform1f("rotateZ", settings->rotateZ->value);
    shader.setUniform1f("rotateW", settings->rotateW->value);
    shader.setUniform1f("fade", settings->fade->value);
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
    return ShaderTypeMountains;
  }

  void drawSettings() override {
    CommonViews::ShaderColor(settings->color);
    CommonViews::ShaderColor(settings->mountainColor);
    CommonViews::ShaderColor(settings->valleyColor);
    CommonViews::ShaderParameter(settings->alpha, nullptr);
    CommonViews::ShaderParameter(settings->heightScale, settings->heightScaleOscillator);
    CommonViews::ShaderParameter(settings->yTranslate, settings->yTranslateOscillator);
    CommonViews::ShaderParameter(settings->zDistance, settings->zDistanceOscillator);
    CommonViews::ShaderParameter(settings->rotateX, settings->rotateXOscillator);
    CommonViews::ShaderParameter(settings->rotateY, settings->rotateYOscillator);
    CommonViews::ShaderParameter(settings->rotateZ, settings->rotateZOscillator);
    CommonViews::ShaderParameter(settings->rotateW, settings->rotateWOscillator);
    CommonViews::ShaderParameter(settings->fade, settings->fadeOscillator);
  }
};

#endif
