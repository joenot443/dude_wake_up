//
//  MotionBlurTextureShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef MotionBlurTextureShader_hpp
#define MotionBlurTextureShader_hpp

#include "ofMain.h"
#include "ShaderSettings.hpp"
#include "Texture.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "WaveformOscillator.hpp"
#include "Parameter.hpp"
#include "TextureService.hpp"
#include "Shader.hpp"
#include <stdio.h>

struct MotionBlurTextureSettings: public ShaderSettings {
  std::shared_ptr<Parameter> shaderValue;
  std::shared_ptr<WaveformOscillator> shaderValueOscillator;
  
  std::shared_ptr<Parameter> texture;
  std::vector<std::string> textureOptions;

  MotionBlurTextureSettings(std::string shaderId, json j) :
  shaderValue(std::make_shared<Parameter>("shaderValue", 0.5, 0.0, 1.0)),
  texture(std::make_shared<Parameter>("texture", 0.0, 0.0, 1000.0)),
  shaderValueOscillator(std::make_shared<WaveformOscillator>(shaderValue)),
  ShaderSettings(shaderId, j, "MotionBlurTexture") {
    parameters = { texture };
    oscillators = { };
    load(j);
    registerParameters();
  };
};

struct MotionBlurTextureShader: Shader {
  MotionBlurTextureSettings *settings;
  MotionBlurTextureShader(MotionBlurTextureSettings *settings) : settings(settings), Shader(settings) {};
  ofShader shader;
  std::shared_ptr<Texture> texture;

  
  void setup() override {
    texture = TextureService::getService()->textureWithName("bark1.png");
    settings->textureOptions = TextureService::getService()->availableTextureNames();
    shader.load("shaders/MotionBlurTexture");
  }

  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override {
    canvas->begin();
    shader.begin();
    if (texture != nullptr) {
      shader.setUniformTexture("tex", texture->fbo.getTexture(), 4);
    }
    shader.setUniform1f("shaderValue", settings->shaderValue->value);
    shader.setUniform1f("time", ofGetElapsedTimef());
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
    return ShaderTypeMotionBlurTexture;
  }

  void drawSettings() override {
    CommonViews::H3Title("MotionBlurTexture");
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

#endif /* MotionBlurTextureShader_hpp */
