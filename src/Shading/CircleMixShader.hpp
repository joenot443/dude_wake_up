//
//  CircleMixShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef CircleMixShader_hpp
#define CircleMixShader_hpp

#include "ofMain.h"
#include "ShaderSettings.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "TimeService.hpp"
#include "WaveformOscillator.hpp"
#include "Parameter.hpp"
#include "Shader.hpp"
#include <stdio.h>

struct CircleMixSettings: public ShaderSettings {
  std::shared_ptr<Parameter> radius;
  std::shared_ptr<WaveformOscillator> radiusOscillator;
  std::shared_ptr<Parameter> warp;
  std::shared_ptr<WaveformOscillator> warpOscillator;
  std::shared_ptr<Parameter> feather;
  std::shared_ptr<WaveformOscillator> featherOscillator;

  CircleMixSettings(std::string shaderId, json j) :
  radius(std::make_shared<Parameter>("radius", 0.5, 0.0, 1.0)),
  radiusOscillator(std::make_shared<WaveformOscillator>(radius)),
  warp(std::make_shared<Parameter>("warp", 1.0, 0.0, 2.0)),
  warpOscillator(std::make_shared<WaveformOscillator>(warp)),
  feather(std::make_shared<Parameter>("feather", 0.1, 0.0, 1.0)),
  featherOscillator(std::make_shared<WaveformOscillator>(feather)),
  ShaderSettings(shaderId, j, "CircleMix") {
    parameters = { radius, warp, feather };
    oscillators = { radiusOscillator, warpOscillator, featherOscillator };
    load(j);
    registerParameters();
  };
};

struct CircleMixShader: Shader {
  CircleMixSettings *settings;
  CircleMixShader(CircleMixSettings *settings) : settings(settings), Shader(settings) {};
  
  void setup() override {
    shader.load("shaders/CircleMix");
  }

  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override {
    canvas->begin();
    frame->draw(0, 0);
    shader.begin();
    
    // Set the first input texture (background)
    shader.setUniformTexture("tex", frame->getTexture(), 4);
    
    // Set the second input texture (circle overlay)

    if (hasInputAtSlot(InputSlotTwo)) {
      auto input2 = inputAtSlot(InputSlotTwo);
      shader.setUniformTexture("tex2", input2->frame()->getTexture(), 5);
    } else {
      // If no second input, use the first input as fallback
      shader.setUniformTexture("tex2", frame->getTexture(), 5);
    }
    
    shader.setUniform1f("radius", settings->radius->value);
    shader.setUniform1f("warp", settings->warp->value);
    shader.setUniform1f("feather", settings->feather->value);
    shader.setUniform1f("time", TimeService::getService()->timeParam->value);
    shader.setUniform2f("dimensions", frame->getWidth(), frame->getHeight());
    frame->draw(0, 0);
    shader.end();
    canvas->end();
  }

  void clear() override {

  }

  int inputCount() override {
    return 2;
  }

  ShaderType type() override {
    return ShaderTypeCircleMix;
  }

  void drawSettings() override {
    CommonViews::H3Title("CircleMix");
    
    CommonViews::ShaderParameter(settings->radius, settings->radiusOscillator);
    CommonViews::ShaderParameter(settings->warp, settings->warpOscillator);
    CommonViews::ShaderParameter(settings->feather, settings->featherOscillator);
  }
};

#endif /* CircleMixShader_hpp */
