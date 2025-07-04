//
//  DirtyPlasmaShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef DirtyPlasmaShader_hpp
#define DirtyPlasmaShader_hpp

#include "ofMain.h"
#include "ShaderSettings.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "ValueOscillator.hpp"
#include "Parameter.hpp"
#include "TextureService.hpp"
#include "Shader.hpp"
#include <stdio.h>
#include "Texture.hpp"

struct DirtyPlasmaSettings: public ShaderSettings {
  std::shared_ptr<Parameter> texture;
  std::vector<std::string> textureOptions;

  std::shared_ptr<Parameter> redMultiplier;
  std::shared_ptr<Parameter> greenMultiplier;
  std::shared_ptr<Parameter> blueMultiplier;
  std::shared_ptr<WaveformOscillator> redMultiplierOscillator;
  std::shared_ptr<WaveformOscillator> greenMultiplierOscillator;
  std::shared_ptr<WaveformOscillator> blueMultiplierOscillator;
  
  // Speed Parameter
  std::shared_ptr<Parameter> speed;
  std::shared_ptr<WaveformOscillator> speedOscillator;
  
  DirtyPlasmaSettings(std::string shaderId, json j) :
  texture(std::make_shared<Parameter>("Texture", 0.0, 0.0, 1000.0)),
  speed(std::make_shared<Parameter>("Speed", 1.0, 0.0, 2.0)),
  redMultiplier(std::make_shared<Parameter>("Red Multiplier", 1.0, 0.0, 2.0)),
  greenMultiplier(std::make_shared<Parameter>("Green Multiplier", 1.0, 0.0, 2.0)),
  blueMultiplier(std::make_shared<Parameter>("Blue Multiplier", 1.0, 0.0, 2.0)),
  redMultiplierOscillator(std::make_shared<WaveformOscillator>(redMultiplier)),
  greenMultiplierOscillator(std::make_shared<WaveformOscillator>(greenMultiplier)),
  blueMultiplierOscillator(std::make_shared<WaveformOscillator>(blueMultiplier)),  
  ShaderSettings(shaderId, j, "DirtyPlasma") {
    parameters = { texture, redMultiplier, greenMultiplier, blueMultiplier };
    oscillators = { redMultiplierOscillator, greenMultiplierOscillator, blueMultiplierOscillator };
    load(j);
    registerParameters();
  };
};

struct DirtyPlasmaShader: Shader {
  DirtyPlasmaSettings *settings;
  std::shared_ptr<Texture> texture;

  DirtyPlasmaShader(DirtyPlasmaSettings *settings) : settings(settings), Shader(settings) {};
  ofShader shader;
  void setup() override {
    texture = TextureService::getService()->textureWithName("Granite");
    settings->textureOptions = TextureService::getService()->availableTextureNames();
    shader.load("shaders/DirtyPlasma");
  }

  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override {
    canvas->begin();
    shader.begin();
    if (texture != nullptr) {
      shader.setUniformTexture("tex", texture->fbo.getTexture(), 4);
    }
    shader.setUniform1f("time", TimeService::getService()->timeParam->value);
    shader.setUniform1f("speed", settings->speed->value);
    shader.setUniform1f("redMultiplier", settings->redMultiplier->value);
    shader.setUniform1f("greenMultiplier", settings->greenMultiplier->value);
    shader.setUniform1f("blueMultiplier", settings->blueMultiplier->value);
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
    return ShaderTypeDirtyPlasma;
  }

  void drawSettings() override {
    
    if (CommonViews::ShaderOption(settings->texture, settings->textureOptions))
    {
      if (settings->texture->value != 0) {
        texture = TextureService::getService()->textureWithName(settings->textureOptions[settings->texture->intValue]);
      } else {
        texture = nullptr;
      }
    }
    CommonViews::ShaderParameter(settings->speed, settings->speedOscillator);
    CommonViews::ShaderParameter(settings->redMultiplier, settings->redMultiplierOscillator);
    CommonViews::ShaderParameter(settings->greenMultiplier, settings->greenMultiplierOscillator);
    CommonViews::ShaderParameter(settings->blueMultiplier, settings->blueMultiplierOscillator);
  }
};

#endif
