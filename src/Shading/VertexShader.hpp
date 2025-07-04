//
//  VertexShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef VertexShader_hpp
#define VertexShader_hpp

#include "ofMain.h"
#include "ShaderSettings.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "ValueOscillator.hpp"
#include "Parameter.hpp"
#include "Shader.hpp"
#include <stdio.h>

struct VertexSettings: public ShaderSettings {
  std::shared_ptr<Parameter> color;
  std::shared_ptr<Parameter> layerCount;
  std::shared_ptr<Parameter> netSpeed;
  std::shared_ptr<Parameter> sparkleIntensity;
  std::shared_ptr<Parameter> lineThickness;
  std::shared_ptr<Parameter> netRotation;
  std::shared_ptr<Parameter> glowStrength;
  std::shared_ptr<Parameter> colorMode;
  std::shared_ptr<Parameter> pulseFrequency;
  std::shared_ptr<Parameter> netScale;
  std::shared_ptr<Parameter> edgeFade;

  VertexSettings(std::string shaderId, json j, std::string name) :
    color(std::make_shared<Parameter>("Color", 1.0, -1.0, 2.0, ParameterType_Color)),
    layerCount(std::make_shared<Parameter>("Layer Count", 4.0, 1.0, 12.0)),
    netSpeed(std::make_shared<Parameter>("Net Speed", 1.0, 0.01, 5.0)),
    sparkleIntensity(std::make_shared<Parameter>("Sparkle Intensity", 1.0, 0.0, 5.0)),
    lineThickness(std::make_shared<Parameter>("Line Thickness", 1.0, 0.1, 5.0)),
    netRotation(std::make_shared<Parameter>("Net Rotation", 0.0, -6.28, 6.28)),
    glowStrength(std::make_shared<Parameter>("Glow Strength", 2.0, 0.0, 10.0)),
    colorMode(std::make_shared<Parameter>("Color Mode", 0.0, 0.0, 2.0, ParameterType_Int)),
    pulseFrequency(std::make_shared<Parameter>("Pulse Frequency", 5.0, 0.1, 20.0)),
    netScale(std::make_shared<Parameter>("Net Scale", 1.0, 0.1, 5.0)),
    edgeFade(std::make_shared<Parameter>("Edge Fade", 1.0, 0.0, 5.0)),
    ShaderSettings(shaderId, j, name)  {
    parameters = { color, layerCount, netSpeed, sparkleIntensity, lineThickness, netRotation, glowStrength, colorMode, pulseFrequency, netScale, edgeFade };
    oscillators = { };
    load(j);
    registerParameters();
  };
};

struct VertexShader: Shader {
  VertexSettings *settings;
  VertexShader(VertexSettings *settings) : settings(settings), Shader(settings) {};

  void setup() override {
    shader.load("shaders/Vertex");
  }

  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override {
    canvas->begin();
    shader.begin();
    shader.setUniformTexture("tex", frame->getTexture(), 4);
    shader.setUniform3f("color", settings->color->color->data()[0], settings->color->color->data()[1], settings->color->color->data()[2]);
    shader.setUniform1f("time", TimeService::getService()->timeParam->value);
    shader.setUniform2f("dimensions", frame->getWidth(), frame->getHeight());
    shader.setUniform1f("layerCount", settings->layerCount->value);
    shader.setUniform1f("netSpeed", settings->netSpeed->value * 0.1);
    shader.setUniform1f("sparkleIntensity", settings->sparkleIntensity->value);
    shader.setUniform1f("lineThickness", settings->lineThickness->value);
    shader.setUniform1f("netRotation", settings->netRotation->value);
    shader.setUniform1f("glowStrength", settings->glowStrength->value);
    shader.setUniform1i("colorMode", settings->colorMode->intValue);
    shader.setUniform1f("pulseFrequency", settings->pulseFrequency->value);
    shader.setUniform1f("netScale", settings->netScale->value);
    shader.setUniform1f("edgeFade", settings->edgeFade->value);
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
    return ShaderTypeVertex;
  }

  void drawSettings() override {
    CommonViews::ShaderColor(settings->color);
    CommonViews::ShaderParameter(settings->layerCount, nullptr);
    CommonViews::ShaderParameter(settings->netSpeed, nullptr);
    CommonViews::ShaderParameter(settings->sparkleIntensity, nullptr);
    CommonViews::ShaderParameter(settings->lineThickness, nullptr);
    CommonViews::ShaderParameter(settings->netRotation, nullptr);
    CommonViews::ShaderParameter(settings->glowStrength, nullptr);
    CommonViews::ShaderIntParameter(settings->colorMode);
    CommonViews::ShaderParameter(settings->pulseFrequency, nullptr);
    CommonViews::ShaderParameter(settings->netScale, nullptr);
    CommonViews::ShaderParameter(settings->edgeFade, nullptr);
  }
};

#endif
