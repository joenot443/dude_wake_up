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
  std::shared_ptr<Parameter> amplitude;
  std::shared_ptr<WaveformOscillator> amplitudeOscillator;

  std::shared_ptr<Parameter> speed;
  std::shared_ptr<WaveformOscillator> speedOscillator;
  
  std::shared_ptr<Parameter> texture;
  std::vector<std::string> textureOptions;

  GlitchRGBSettings(std::string shaderId, json j) :
  amplitude(std::make_shared<Parameter>("Amplitude", 0.1, 0.0, 1.0)),
  amplitudeOscillator(std::make_shared<WaveformOscillator>(amplitude)),
  speed(std::make_shared<Parameter>("Speed", 0.1, 0.0, 1.0)),
  speedOscillator(std::make_shared<WaveformOscillator>(speed)),
  texture(std::make_shared<Parameter>("Texture", 0.0, 0.0, 1000.0)),

  ShaderSettings(shaderId, j, "GlitchRGB") {
    parameters = { amplitude, speed, texture };
    oscillators = { amplitudeOscillator, speedOscillator };
    load(j);
    registerParameters();
  };
};

struct GlitchRGBShader: Shader {
  GlitchRGBSettings *settings;
  GlitchRGBShader(GlitchRGBSettings *settings) : settings(settings), Shader(settings) {};
  std::shared_ptr<Texture> texture;

  
  void setup() override {
    texture = TextureService::getService()->textureWithName("Gravel Path");
    settings->textureOptions = TextureService::getService()->availableTextureNames();
    shader.load("shaders/Glitch RGB");
  }

  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override {
    canvas->begin();
    shader.begin();
    shader.setUniformTexture("tex", frame->getTexture(), 4);
    if (texture != nullptr) {
      shader.setUniformTexture("tex2", texture->fbo.getTexture(), 8);
    }
    shader.setUniform1f("amplitude", settings->amplitude->value);
    shader.setUniform1f("speed", settings->speed->value);
    shader.setUniform1f("time", TimeService::getService()->timeParam->value);
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
    CommonViews::ShaderParameter(settings->amplitude, settings->amplitudeOscillator);
    CommonViews::ShaderParameter(settings->speed, settings->speedOscillator);
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
