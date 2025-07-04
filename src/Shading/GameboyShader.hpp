//
//  GameboyShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef GameboyShader_hpp
#define GameboyShader_hpp

#include "ofMain.h"
#include "ShaderSettings.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "ValueOscillator.hpp"
#include "Parameter.hpp"
#include "ImageTexture.hpp"
#include "Shader.hpp"
#include <stdio.h>

struct GameboySettings: public ShaderSettings {
  std::shared_ptr<Parameter> texture;

  std::shared_ptr<Parameter> scale;
  std::shared_ptr<WaveformOscillator> scaleOscillator;
  
  std::shared_ptr<Parameter> mode;
  std::vector<std::string> textureOptions;

  GameboySettings(std::string shaderId, json j, std::string name) :
  scale(std::make_shared<Parameter>("Scale", 1.0, 0.0, 5.0)),
  texture(std::make_shared<Parameter>("Texture", 0.0, 0.0, 1000.0)),
  mode(std::make_shared<Parameter>("Color", 1.0, 0.0, 1.0, ParameterType_Bool)),
  scaleOscillator(std::make_shared<WaveformOscillator>(scale)),
  textureOptions({}),
  ShaderSettings(shaderId, j, name) {
    parameters = { };
    oscillators = { };
    load(j);
    audioReactiveParameter = scale;
    registerParameters();
  };
};

struct GameboyShader: Shader {
  GameboySettings *settings;
  std::shared_ptr<Texture> texture;

  GameboyShader(GameboySettings *settings) : 
  settings(settings),
  Shader(settings) {};

  void setup() override {
    shader.load("shaders/Gameboy");
    settings->textureOptions = TextureService::getService()->availableTextureNames();
    settings->textureOptions.insert(settings->textureOptions.begin(), "No Texture");
  }

  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override {
    canvas->begin();
    shader.begin();
    // Clear the frame
    ofClear(0,0,0, 255);
    ofClear(0,0,0, 0);
    shader.setUniformTexture("tex", frame->getTexture(), 4);
    if (texture != nullptr) {
      shader.setUniformTexture("tex2", texture->fbo.getTexture(), 8);
    }
    shader.setUniform1i("mode", settings->mode->boolValue);
    shader.setUniform1f("time", TimeService::getService()->timeParam->value);
    shader.setUniform1f("scale", settings->scale->value);
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
    return ShaderTypeGameboy;
  }

  void drawSettings() override {
    
    CommonViews::ShaderCheckbox(settings->mode);
    CommonViews::ShaderParameter(settings->scale, settings->scaleOscillator);
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
