//
//  GlitchDigitalShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef GlitchDigitalShader_hpp
#define GlitchDigitalShader_hpp

#include "ofMain.h"
#include "ShaderSettings.hpp"
#include "CommonViews.hpp"
#include "Texture.hpp"
#include "ofxImGui.h"
#include "WaveformOscillator.hpp"
#include "Parameter.hpp"
#include "TextureService.hpp"
#include "Shader.hpp"
#include <stdio.h>

struct GlitchDigitalSettings: public ShaderSettings {
  std::shared_ptr<Parameter> shaderValue;
  std::shared_ptr<WaveformOscillator> shaderValueOscillator;
  
  std::shared_ptr<Parameter> texture;
  std::vector<std::string> textureOptions;

  std::shared_ptr<Parameter> amount;
  std::shared_ptr<WaveformOscillator> amountOscillator;

  std::shared_ptr<Parameter> threshold;
  std::shared_ptr<WaveformOscillator> thresholdOscillator;

  GlitchDigitalSettings(std::string shaderId, json j) :
  shaderValue(std::make_shared<Parameter>("shaderValue", 0.5, 0.0, 1.0)),
  shaderValueOscillator(std::make_shared<WaveformOscillator>(shaderValue)),

  amount(std::make_shared<Parameter>("amount", 0.5, 0.0, 1.0)),
  amountOscillator(std::make_shared<WaveformOscillator>(amount)),

  threshold(std::make_shared<Parameter>("threshold", 0.5, 0.0, 1.0)),
  thresholdOscillator(std::make_shared<WaveformOscillator>(threshold)),

  texture(std::make_shared<Parameter>("texture", 0.0, 0.0, 1000.0)),
  ShaderSettings(shaderId, j, "GlitchDigital") {
    parameters = { texture, amount, threshold };
    oscillators = { amountOscillator, thresholdOscillator };
    load(j);
    registerParameters();
  };
};

struct GlitchDigitalShader: Shader {
  GlitchDigitalSettings *settings;
  GlitchDigitalShader(GlitchDigitalSettings *settings) : settings(settings), Shader(settings) {};
  std::shared_ptr<Texture> texture;
  
  void setup() override {
    texture = TextureService::getService()->textureWithName("bark1.png");
    settings->textureOptions = TextureService::getService()->availableTextureNames();
    shader.load("shaders/GlitchDigital");
  }

  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override {
    canvas->begin();
    shader.begin();
    shader.setUniformTexture("tex", frame->getTexture(), 4);
    if (texture != nullptr) {
      shader.setUniformTexture("tex2", texture->fbo.getTexture(), 8);
    }
    shader.setUniform1f("shaderValue", settings->shaderValue->value);
    shader.setUniform1f("time", ofGetElapsedTimef());
    shader.setUniform1f("amount", settings->amount->value);
    shader.setUniform1f("threshold", settings->threshold->value);
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
    return ShaderTypeGlitchDigital;
  }

  void drawSettings() override {
    CommonViews::ShaderParameter(settings->amount, settings->amountOscillator);
    CommonViews::ShaderParameter(settings->threshold, settings->thresholdOscillator);
    if (CommonViews::ShaderOption(settings->texture, settings->textureOptions))
    {
      if (settings->texture->value != 0) {
        texture = TextureService::getService()->textureWithName(settings->textureOptions[settings->texture->intValue]);
      } else {
        texture = nullptr;
      }
    }
  }
};

#endif
