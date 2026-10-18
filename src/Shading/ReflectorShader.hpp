//
//  ReflectorShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef ReflectorShader_hpp
#define ReflectorShader_hpp

#include "ofMain.h"
#include "ShaderSettings.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "ValueOscillator.hpp"
#include "Parameter.hpp"
#include "Shader.hpp"
#include <stdio.h>

struct ReflectorSettings: public ShaderSettings {
  std::shared_ptr<Parameter> rotationSpeed;
  std::shared_ptr<Parameter> polyZoom;
  std::shared_ptr<Parameter> innerSphere;
  std::shared_ptr<Parameter> refrIndex;
  std::shared_ptr<Parameter> maxBounces;
  std::shared_ptr<Parameter> polyType;

  std::shared_ptr<WaveformOscillator> rotationSpeedOscillator;
  std::shared_ptr<WaveformOscillator> polyZoomOscillator;
  std::shared_ptr<WaveformOscillator> innerSphereOscillator;
  std::shared_ptr<WaveformOscillator> refrIndexOscillator;

  ReflectorSettings(std::string shaderId, json j) :
  rotationSpeed(std::make_shared<Parameter>("Rotation Speed", 0.25, 0.0, 2.0)),
  polyZoom(std::make_shared<Parameter>("Zoom", 2.0, 0.5, 6.0)),
  innerSphere(std::make_shared<Parameter>("Inner Sphere", 1.0, 0.0, 2.0)),
  refrIndex(std::make_shared<Parameter>("Refraction", 0.9, 0.5, 1.5)),
  maxBounces(std::make_shared<Parameter>("Bounces", 6, 1, 10)),
  polyType(std::make_shared<Parameter>("Symmetry", 3, 2, 5, ParameterType_Int)),
  rotationSpeedOscillator(std::make_shared<WaveformOscillator>(rotationSpeed)),
  polyZoomOscillator(std::make_shared<WaveformOscillator>(polyZoom)),
  innerSphereOscillator(std::make_shared<WaveformOscillator>(innerSphere)),
  refrIndexOscillator(std::make_shared<WaveformOscillator>(refrIndex)),
  ShaderSettings(shaderId, j, "Reflector") {
    parameters = { rotationSpeed, polyZoom, innerSphere, refrIndex, maxBounces, polyType };
    oscillators = { rotationSpeedOscillator, polyZoomOscillator, innerSphereOscillator, refrIndexOscillator };
    load(j);
    registerParameters();
  };
};

struct ReflectorShader: Shader {
  ReflectorSettings *settings;
  ReflectorShader(ReflectorSettings *settings) : settings(settings), Shader(settings) {};
  void setup() override {
    shader.load("shaders/Reflector");
  }

  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override {
    canvas->begin();
    shader.begin();
    shader.setUniformTexture("tex", frame->getTexture(), 4);
    shader.setUniform1f("rotation_speed", settings->rotationSpeed->value);
    shader.setUniform1f("poly_zoom", settings->polyZoom->value);
    shader.setUniform1f("inner_sphere", settings->innerSphere->value);
    shader.setUniform1f("refr_index", settings->refrIndex->value);
    shader.setUniform1i("max_bounces", settings->maxBounces->intValue);
    shader.setUniform1i("poly_type", settings->polyType->intValue);
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
    return ShaderTypeReflector;
  }

  void drawSettings() override {
    CommonViews::ShaderParameter(settings->rotationSpeed, settings->rotationSpeedOscillator);
    CommonViews::ShaderParameter(settings->polyZoom, settings->polyZoomOscillator);
    CommonViews::ShaderParameter(settings->innerSphere, settings->innerSphereOscillator);
    CommonViews::ShaderParameter(settings->refrIndex, settings->refrIndexOscillator);
    CommonViews::ShaderIntParameter(settings->maxBounces);
    CommonViews::ShaderIntParameter(settings->polyType);
  }
};

#endif
