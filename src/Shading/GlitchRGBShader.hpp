//
//  GlitchRGBShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef GlitchRGBShader_hpp
#define GlitchRGBShader_hpp

#include "ofMain.h"
#include "Texture.hpp"
#include "TextureService.hpp"
#include "ShaderSettings.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "WaveformOscillator.hpp"
#include "Parameter.hpp"
#include "Shader.hpp"
#include <stdio.h>

struct GlitchRGBSettings: public ShaderSettings {
  std::shared_ptr<Parameter> shaderValue;
  std::shared_ptr<WaveformOscillator> shaderValueOscillator;
  
  std::shared_ptr<Parameter> texture;
  std::vector<std::string> textureOptions;

  GlitchRGBSettings(std::string shaderId, json j) :
  shaderValue(std::make_shared<Parameter>("shaderValue", 0.5, 0.0, 1.0)),
  shaderValueOscillator(std::make_shared<WaveformOscillator>(shaderValue)),
  texture(std::make_shared<Parameter>("texture", 0.0, 0.0, 1000.0)),

  ShaderSettings(shaderId, j, "GlitchRGB") {
    parameters = { texture };
    oscillators = { };
    load(j);
    registerParameters();
  };
};

struct GlitchRGBShader: Shader {
  GlitchRGBSettings *settings;
  GlitchRGBShader(GlitchRGBSettings *settings) : settings(settings), Shader(settings) {};
  std::shared_ptr<Texture> texture;

  
  void setup() override {
    texture = TextureService::getService()->textureWithName("wornpaintedcement.png");
    settings->textureOptions = TextureService::getService()->availableTextureNames();
    shader.load("shaders/GlitchRGB");
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
    return ShaderTypeGlitchRGB;
  }

  void drawSettings() override {
    CommonViews::H3Title("GlitchRGB");

    CommonViews::ShaderParameter(settings->shaderValue, settings->shaderValueOscillator);
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

#endif /* GlitchRGBShader_hpp */
