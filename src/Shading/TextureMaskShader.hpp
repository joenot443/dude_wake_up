//
//  TextureMaskShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef TextureMaskShader_hpp
#define TextureMaskShader_hpp

#include "ofMain.h"
#include "Texture.hpp"
#include "TextureService.hpp"
#include "ShaderSettings.hpp"
#include "TextureService.hpp"
#include "ConfigService.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "ValueOscillator.hpp"
#include "Parameter.hpp"
#include "Shader.hpp"
#include <stdio.h>

struct TextureMaskSettings: public ShaderSettings {
  std::shared_ptr<Parameter> texture;
  std::shared_ptr<Parameter> blend;
  std::shared_ptr<WaveformOscillator> blendOscillator;
  std::vector<std::string> textureOptions;
  
  TextureMaskSettings(std::string shaderId, json j, std::string name) :
  texture(std::make_shared<Parameter>("texture", 0.0, 0.0, 1000.0)),
  blend(std::make_shared<Parameter>("blend", 1.0, 0.0, 1.0)),
  blendOscillator(std::make_shared<WaveformOscillator>(blend)),
  ShaderSettings(shaderId, j, name)  {
    parameters = { texture, blend };
    oscillators = { blendOscillator };
    load(j);
    registerParameters();
  };
};

struct TextureMaskShader: Shader {
  TextureMaskSettings *settings;
    std::shared_ptr<Texture> texture;
  TextureMaskShader(TextureMaskSettings *settings) : settings(settings), Shader(settings) {};
  ofShader shader;
  void setup() override {
    shader.load("shaders/Mask");
    settings->textureOptions = TextureService::getService()->availableTextureNames();
    settings->textureOptions.insert(settings->textureOptions.begin(), "No Texture");
  }

  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override {
    canvas->begin();
    shader.begin();
    if (texture != nullptr) {
      shader.setUniformTexture("tex", texture->fbo.getTexture(), 4);
    }
    shader.setUniformTexture("maskTex", frame->getTexture(), 12);
    shader.setUniform1f("time", ofGetElapsedTimef());
    shader.setUniform1f("blend", settings->blend->value);
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
    return ShaderTypeTextureMask;
  }

  void drawSettings() override {
    CommonViews::H3Title("TextureMask");

    if (CommonViews::ShaderOption(settings->texture, settings->textureOptions))
    {
      if (settings->texture->value != 0) {
        texture = TextureService::getService()->textureWithName(settings->textureOptions[settings->texture->intValue]);
      } else {
        texture = nullptr;
      }
    }
    CommonViews::ShaderParameter(settings->blend, settings->blendOscillator);
  }
};

#endif /* TextureMaskShader_hpp */
