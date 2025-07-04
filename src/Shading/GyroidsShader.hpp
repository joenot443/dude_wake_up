//
//  GyroidsShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef GyroidsShader_hpp
#define GyroidsShader_hpp

#include "ofMain.h"
#include "ShaderSettings.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "WaveformOscillator.hpp"
#include "Parameter.hpp"
#include "Shader.hpp"
#include <stdio.h>

struct GyroidsSettings: public ShaderSettings {
  std::shared_ptr<Parameter> paletteMorph;
  std::shared_ptr<WaveformOscillator> paletteMorphOscillator;

  std::shared_ptr<Parameter> gyroidComplexity;
  std::shared_ptr<WaveformOscillator> gyroidComplexityOscillator;

  std::shared_ptr<Parameter> surfaceDistortion;
  std::shared_ptr<WaveformOscillator> surfaceDistortionOscillator;

  std::shared_ptr<Parameter> maxSteps;
  std::shared_ptr<WaveformOscillator> maxStepsOscillator;

  std::shared_ptr<Parameter> aoIntensity;
  std::shared_ptr<WaveformOscillator> aoIntensityOscillator;

  std::shared_ptr<Parameter> fresnel;

  GyroidsSettings(std::string shaderId, json j, std::string name) :
  paletteMorph(std::make_shared<Parameter>("Palette Morph", 0.0, 0.0, 1.0)),
  gyroidComplexity(std::make_shared<Parameter>("Gyroid Complexity", 1.0, 0.1, 5.0)),
  surfaceDistortion(std::make_shared<Parameter>("Surface Distortion", 0.0, 0.0, 0.5)),
  maxSteps(std::make_shared<Parameter>("Max Steps", 100, 20, 200)),
  aoIntensity(std::make_shared<Parameter>("AO Intensity", 1.0, 0.0, 2.0)),
  fresnel(std::make_shared<Parameter>("Fresnel", ParameterType_Color)), 
  paletteMorphOscillator(std::make_shared<WaveformOscillator>(paletteMorph)),
  gyroidComplexityOscillator(std::make_shared<WaveformOscillator>(gyroidComplexity)),
  surfaceDistortionOscillator(std::make_shared<WaveformOscillator>(surfaceDistortion)),
  maxStepsOscillator(std::make_shared<WaveformOscillator>(maxSteps)),
  aoIntensityOscillator(std::make_shared<WaveformOscillator>(aoIntensity)),
  ShaderSettings(shaderId, j, name)  {
    parameters = { paletteMorph, gyroidComplexity, surfaceDistortion, maxSteps, aoIntensity, fresnel };
    oscillators = { paletteMorphOscillator, gyroidComplexityOscillator, surfaceDistortionOscillator, maxStepsOscillator, aoIntensityOscillator };
    load(j);
    registerParameters();
  };
};

struct GyroidsShader: Shader {
  GyroidsSettings *settings;
  GyroidsShader(GyroidsSettings *settings) : settings(settings), Shader(settings) {};

  void setup() override {
    shader.load("shaders/Gyroids");
  }
  
  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override {
    canvas->begin();
    shader.begin();
    shader.setUniformTexture("tex", frame->getTexture(), 4);
    shader.setUniform1f("time", TimeService::getService()->timeParam->value);
    shader.setUniform2f("dimensions", frame->getWidth(), frame->getHeight());
    shader.setUniform1f("paletteMorph", settings->paletteMorph->value);
    shader.setUniform1f("gyroidComplexity", settings->gyroidComplexity->value);
    shader.setUniform1f("surfaceDistortion", settings->surfaceDistortion->value);
    shader.setUniform1i("maxSteps", settings->maxSteps->intValue);
    shader.setUniform1f("aoIntensity", settings->aoIntensity->value);
    shader.setUniform3f("fresnelColor", settings->fresnel->color->data()[0], settings->fresnel->color->data()[1], settings->fresnel->color->data()[2]);
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
    return ShaderTypeGyroids;
  }
  
  void drawSettings() override {
    CommonViews::ShaderParameter(settings->paletteMorph, settings->paletteMorphOscillator);
    CommonViews::ShaderParameter(settings->gyroidComplexity, settings->gyroidComplexityOscillator);
    CommonViews::ShaderParameter(settings->surfaceDistortion, settings->surfaceDistortionOscillator);
    
    CommonViews::H2Title("Quality & Lighting");
    CommonViews::ShaderParameter(settings->maxSteps, settings->maxStepsOscillator);
    CommonViews::ShaderParameter(settings->aoIntensity, settings->aoIntensityOscillator);

    CommonViews::H2Title("Fresnel Color");
    CommonViews::ShaderColor(settings->fresnel);
  }
};

#endif
