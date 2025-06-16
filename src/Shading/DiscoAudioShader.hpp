//
//  DiscoAudioShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef DiscoAudioShader_hpp
#define DiscoAudioShader_hpp

#include "ofMain.h"
#include "ShaderSettings.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "AudioSourceService.hpp"
#include "WaveformOscillator.hpp"
#include "Parameter.hpp"
#include "Shader.hpp"
#include <stdio.h>

struct DiscoAudioSettings: public ShaderSettings {
  std::shared_ptr<Parameter> audioDisplacementScale;
  std::shared_ptr<WaveformOscillator> audioDisplacementScaleOscillator;

  std::shared_ptr<Parameter> patternComplexityParam;
  std::shared_ptr<WaveformOscillator> patternComplexityOscillator;

  // New Color Parameters
  std::shared_ptr<Parameter> colorPaletteSeedParam;
  std::shared_ptr<WaveformOscillator> colorPaletteSeedOscillator;

  std::shared_ptr<Parameter> glowTint;

  std::shared_ptr<Parameter> objectSaturationParam;
  std::shared_ptr<WaveformOscillator> objectSaturationOscillator;


  DiscoAudioSettings(std::string shaderId, json j) :
  audioDisplacementScale(std::make_shared<Parameter>("Audio Scale", 1.0, 0.0, 5.0)),
  audioDisplacementScaleOscillator(std::make_shared<WaveformOscillator>(audioDisplacementScale)),
  patternComplexityParam(std::make_shared<Parameter>("Complexity", 4.0, 1.0, 10.0)),
  patternComplexityOscillator(std::make_shared<WaveformOscillator>(patternComplexityParam)),
  // Initialize new color parameters
  colorPaletteSeedParam(std::make_shared<Parameter>("Palette Seed", 0.0, 0.0, 100.0)),
  colorPaletteSeedOscillator(std::make_shared<WaveformOscillator>(colorPaletteSeedParam)),
  glowTint(std::make_shared<Parameter>("Glow Tint", ParameterType_Color)),
  objectSaturationParam(std::make_shared<Parameter>("Saturation", 1.0, 0.0, 2.0)),
  objectSaturationOscillator(std::make_shared<WaveformOscillator>(objectSaturationParam)),
  ShaderSettings(shaderId, j, "DiscoAudio") {
    parameters = { audioDisplacementScale, patternComplexityParam,
                   colorPaletteSeedParam, glowTint, objectSaturationParam };
    oscillators = { audioDisplacementScaleOscillator, patternComplexityOscillator,
                    colorPaletteSeedOscillator, objectSaturationOscillator /*, add glow oscillators if created */ };
    load(j);
    registerParameters();
  };
};

struct DiscoAudioShader: Shader {
  DiscoAudioSettings *settings;
  DiscoAudioShader(DiscoAudioSettings *settings) : settings(settings), Shader(settings) {};
  
  void setup() override {
    shader.load("shaders/Disco Audio");
  }

  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override {
    auto source = AudioSourceService::getService()->selectedAudioSource;

    canvas->begin();
    shader.begin();
    if (source != nullptr && source->audioAnalysis.smoothSpectrum.size() > 0)
      shader.setUniform1fv("audio", &source->audioAnalysis.smoothSpectrum[0],
                           256);
    shader.setUniformTexture("tex", frame->getTexture(), 4); 
    shader.setUniform1f("time", ofGetElapsedTimef());
    shader.setUniform2f("dimensions", frame->getWidth(), frame->getHeight());
    shader.setUniform1f("audioDisplacementScale", settings->audioDisplacementScale->value);
    shader.setUniform1i("patternComplexity", settings->patternComplexityParam->intValue);
    
    // Pass new color uniforms
    shader.setUniform1f("colorPaletteSeed", settings->colorPaletteSeedParam->value);
    shader.setUniform3f("glowTintColor", settings->glowTint->color->data()[0], settings->glowTint->color->data()[1], settings->glowTint->color->data()[2]);
    shader.setUniform1f("objectSaturation", settings->objectSaturationParam->value);
    
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
    return ShaderTypeDiscoAudio;
  }

  void drawSettings() override {
    CommonViews::ShaderParameter(settings->audioDisplacementScale, settings->audioDisplacementScaleOscillator);
    CommonViews::ShaderParameter(settings->patternComplexityParam, settings->patternComplexityOscillator);
    
    // Draw UI for new color parameters
    CommonViews::ShaderParameter(settings->colorPaletteSeedParam, settings->colorPaletteSeedOscillator);
    // For glow tint, you might want a color picker in the future, or group these.
    CommonViews::ShaderColor(settings->glowTint);
    CommonViews::ShaderParameter(settings->objectSaturationParam, settings->objectSaturationOscillator);
  }
};

#endif  
