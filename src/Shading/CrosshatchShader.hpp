//
//  CrosshatchShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef CrosshatchShader_hpp
#define CrosshatchShader_hpp

#include "ofMain.h"
#include "ShaderSettings.hpp"
#include "CommonViews.hpp"
#include "Shader.hpp"
#include <stdio.h>

struct CrosshatchSettings: public ShaderSettings {
  std::shared_ptr<Parameter> mix;
  std::shared_ptr<Oscillator> mixOscillator;
  
  std::shared_ptr<Parameter> texture;
  std::vector<std::string> textureOptions;

  // New uniforms
  std::shared_ptr<Parameter> outlineStyleExponentBase;
  std::shared_ptr<Oscillator> outlineStyleExponentBaseOscillator;
  std::shared_ptr<Parameter> hatchAngleQuadraticFactor;
  std::shared_ptr<Oscillator> hatchAngleQuadraticFactorOscillator;
  std::shared_ptr<Parameter> hatchSharpness;
  std::shared_ptr<Oscillator> hatchSharpnessOscillator;
  std::shared_ptr<Parameter> paperBaseColorR;
  std::shared_ptr<Oscillator> paperBaseColorROscillator;
  std::shared_ptr<Parameter> paperBaseColorG;
  std::shared_ptr<Oscillator> paperBaseColorGOscillator;
  std::shared_ptr<Parameter> paperBaseColorB;
  std::shared_ptr<Oscillator> paperBaseColorBOscillator;
  std::shared_ptr<Parameter> noiseWiggleAmount;
  std::shared_ptr<Oscillator> noiseWiggleAmountOscillator;
  
	public:
  CrosshatchSettings(std::string shaderId, json j, std::string name) :
  mix(std::make_shared<Parameter>("Amount", 1.0, 0.0, 10.0)),
  texture(std::make_shared<Parameter>("texture", 0.0, 0.0, 1000.0)),
  mixOscillator(std::make_shared<WaveformOscillator>(mix)),
  outlineStyleExponentBase(std::make_shared<Parameter>("Outline Exponent Base", 1.3, 0.5, 3.0)),
  outlineStyleExponentBaseOscillator(std::make_shared<WaveformOscillator>(outlineStyleExponentBase)),
  hatchAngleQuadraticFactor(std::make_shared<Parameter>("Hatch Angle Quad Factor", 0.08, 0.0, 0.5)),
  hatchAngleQuadraticFactorOscillator(std::make_shared<WaveformOscillator>(hatchAngleQuadraticFactor)),
  hatchSharpness(std::make_shared<Parameter>("Hatch Sharpness", 1.0, 0.01, 5.0)),
  hatchSharpnessOscillator(std::make_shared<WaveformOscillator>(hatchSharpness)),
  paperBaseColorR(std::make_shared<Parameter>("Paper Color R", 0.95, 0.0, 1.0)),
  paperBaseColorROscillator(std::make_shared<WaveformOscillator>(paperBaseColorR)),
  paperBaseColorG(std::make_shared<Parameter>("Paper Color G", 0.95, 0.0, 1.0)),
  paperBaseColorGOscillator(std::make_shared<WaveformOscillator>(paperBaseColorG)),
  paperBaseColorB(std::make_shared<Parameter>("Paper Color B", 0.92, 0.0, 1.0)),
  paperBaseColorBOscillator(std::make_shared<WaveformOscillator>(paperBaseColorB)),
  noiseWiggleAmount(std::make_shared<Parameter>("Noise Wiggle Amount", 10.0, 0.0, 50.0)),
  noiseWiggleAmountOscillator(std::make_shared<WaveformOscillator>(noiseWiggleAmount)),
  ShaderSettings(shaderId, j, name) {
    parameters = { mix, texture, outlineStyleExponentBase, hatchAngleQuadraticFactor, hatchSharpness, paperBaseColorR, paperBaseColorG, paperBaseColorB, noiseWiggleAmount };
    oscillators = { mixOscillator, outlineStyleExponentBaseOscillator, hatchAngleQuadraticFactorOscillator, hatchSharpnessOscillator, paperBaseColorROscillator, paperBaseColorGOscillator, paperBaseColorBOscillator, noiseWiggleAmountOscillator };
    audioReactiveParameter = mix;
    registerParameters();
  };
};

struct CrosshatchShader: Shader {
  CrosshatchSettings *settings;
  std::shared_ptr<Texture> texture;

  CrosshatchShader(CrosshatchSettings *settings) : settings(settings), Shader(settings) {
    texture = TextureService::getService()->textureWithName("bark1.png");
    settings->textureOptions = TextureService::getService()->availableTextureNames();

  };

  void setup() override {
    shader.load("shaders/Crosshatch");
    texture = TextureService::getService()->textureWithName("bark1.png");
    settings->textureOptions = TextureService::getService()->availableTextureNames();
  }

  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override {
    canvas->begin();
    shader.begin();
    ofClear(0,0,0,255);
    ofClear(0,0,0,0);
    shader.setUniformTexture("tex", frame->getTexture(), 4);
    if (texture != nullptr) {
      shader.setUniformTexture("rand", texture->fbo.getTexture(), 8);
    } else {
      // Maybe set a default noise texture if texture is null?
    }
    shader.setUniform1f("time", TimeService::getService()->timeParam->value);
    shader.setUniform1f("amount", settings->mix->value);
    shader.setUniform2f("dimensions", frame->getWidth(), frame->getHeight());

    // Set new uniforms
    shader.setUniform1f("outlineStyleExponentBase", settings->outlineStyleExponentBase->value);
    shader.setUniform1f("hatchAngleQuadraticFactor", settings->hatchAngleQuadraticFactor->value);
    shader.setUniform1f("hatchSharpness", settings->hatchSharpness->value);
    shader.setUniform3f("paperBaseColor", settings->paperBaseColorR->value, settings->paperBaseColorG->value, settings->paperBaseColorB->value);
    shader.setUniform1f("noiseWiggleAmount", settings->noiseWiggleAmount->value);

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
    return ShaderTypeCrosshatch;
  }

  void drawSettings() override {
    if (CommonViews::ShaderOption(settings->texture, settings->textureOptions))
    {
      if (settings->texture->intValue >= 0 && settings->texture->intValue < settings->textureOptions.size()) {
         texture = TextureService::getService()->textureWithName(settings->textureOptions[settings->texture->intValue]);
      } else {
         texture = TextureService::getService()->textureWithName(settings->textureOptions[0]); 
         settings->texture->intValue = 0;
      }
    }
    
    CommonViews::ShaderParameter(settings->mix, settings->mixOscillator);
    
    CommonViews::ShaderParameter(settings->outlineStyleExponentBase, settings->outlineStyleExponentBaseOscillator);
    CommonViews::ShaderParameter(settings->hatchAngleQuadraticFactor, settings->hatchAngleQuadraticFactorOscillator);
    CommonViews::ShaderParameter(settings->hatchSharpness, settings->hatchSharpnessOscillator);
    CommonViews::ShaderParameter(settings->paperBaseColorR, settings->paperBaseColorROscillator);
    CommonViews::ShaderParameter(settings->paperBaseColorG, settings->paperBaseColorGOscillator);
    CommonViews::ShaderParameter(settings->paperBaseColorB, settings->paperBaseColorBOscillator);
    CommonViews::ShaderParameter(settings->noiseWiggleAmount, settings->noiseWiggleAmountOscillator);
  }
};

#endif
