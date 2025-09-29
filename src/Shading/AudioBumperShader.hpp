//
//  AudioBumperShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef AudioBumperShader_hpp
#define AudioBumperShader_hpp

#include "AudioSourceService.hpp"
#include "CommonViews.hpp"
#include "ShaderConfigSelectionView.hpp"
#include "Shader.hpp"
#include "ShaderSettings.hpp"
#include "ofMain.h"
#include "ofxImGui.h"
#include <stdio.h>

struct AudioBumperSettings : public ShaderSettings {
  std::shared_ptr<Parameter> minColor;
  std::shared_ptr<Parameter> maxColor;
  
public:
  AudioBumperSettings(std::string shaderId, json j, std::string name)
  :
  minColor(std::make_shared<Parameter>("Minimum Color", ParameterType_Color)),
  maxColor(std::make_shared<Parameter>("Maximum Color", ParameterType_Color)),
  ShaderSettings(shaderId, j, name){
    minColor->color = std::make_shared<std::array<float, 4>>(std::array<float, 4>({1.0f, 0.5f, 1.0f, 1.0f}));
    maxColor->color = std::make_shared<std::array<float, 4>>(std::array<float, 4>({0.5f, 1.0f, 1.0f, 1.0f}));
    parameters = { minColor, maxColor };
    registerParameters();
    load(j);
  };
};

class AudioBumperShader : public Shader {
public:
  
  AudioBumperSettings *settings;
  AudioBumperShader(AudioBumperSettings *settings)
  : settings(settings), Shader(settings){};

  void setup() override {
    shader.load("shaders/AudioBumper");    
  }
  
  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override {
    auto source = AudioSourceService::getService()->selectedAudioSource;
    canvas->begin();
    shader.begin();
    shader.setUniformTexture("tex", frame->getTexture(), 4);
    if (source != nullptr && source->audioAnalysis.smoothSpectrum.size() > 0)
      shader.setUniform1fv("audio", &source->audioAnalysis.smoothSpectrum[0],
                           256);
    shader.setUniform3f("minColor", settings->minColor->color->data()[0], settings->minColor->color->data()[1], settings->minColor->color->data()[2]);
    shader.setUniform3f("maxColor", settings->maxColor->color->data()[0], settings->maxColor->color->data()[1], settings->maxColor->color->data()[2]);
    // Flip the frame vertically
    ofPushMatrix();
    ofTranslate(0, frame->getHeight());
    ofScale(1, -1);
    frame->draw(0, 0);
    ofPopMatrix();
    shader.end();
    canvas->end();
  }
  
  void clear() override {}
  
  int inputCount() override {
    return 1;
  }
  ShaderType type() override { return ShaderTypeAudioBumper; }
  
  void drawSettings() override {
    
    CommonViews::ShaderColor(settings->minColor);
    CommonViews::ShaderColor(settings->maxColor);
  }
};

#endif
