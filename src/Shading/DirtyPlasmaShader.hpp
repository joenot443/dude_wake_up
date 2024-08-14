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
  
  // Speed Parameter
  std::shared_ptr<Parameter> speed;
  std::shared_ptr<WaveformOscillator> speedOscillator;
  
  DirtyPlasmaSettings(std::string shaderId, json j) :
  texture(std::make_shared<Parameter>("texture", 0.0, 0.0, 1000.0)),
  speed(std::make_shared<Parameter>("speed", 1.0, 0.0, 2.0)),
  ShaderSettings(shaderId, j, "DirtyPlasma") {
    parameters = { texture };
    oscillators = { };
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
    texture = TextureService::getService()->textureWithName("bark1.png");
    settings->textureOptions = TextureService::getService()->availableTextureNames();
    shader.load("shaders/DirtyPlasma");
  }

  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override {
    canvas->begin();
    shader.begin();
    if (texture != nullptr) {
      shader.setUniformTexture("tex", texture->fbo.getTexture(), 4);
    }
    shader.setUniform1f("time", ofGetElapsedTimef());
    shader.setUniform1f("speed", settings->speed->value);
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
    CommonViews::H3Title("DirtyPlasma");
    if (CommonViews::ShaderOption(settings->texture, settings->textureOptions))
    {
      if (settings->texture->value != 0) {
        texture = TextureService::getService()->textureWithName(settings->textureOptions[settings->texture->intValue]);
      } else {
        texture = nullptr;
      }
    }
    CommonViews::ShaderParameter(settings->speed, settings->speedOscillator);
  }
};

#endif /* DirtyPlasmaShader_hpp */
