//
//  RisoColumnsShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef RisoColumnsShader_hpp
#define RisoColumnsShader_hpp

#include "ofMain.h"
#include "ShaderSettings.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "TimeService.hpp"
#include "WaveformOscillator.hpp"
#include "Parameter.hpp"
#include "Shader.hpp"
#include <stdio.h>

struct RisoColumnsSettings: public ShaderSettings {
  std::shared_ptr<Parameter> speed;
  std::shared_ptr<WaveformOscillator> speedOscillator;

  std::shared_ptr<Parameter> columns;
  std::shared_ptr<WaveformOscillator> columnsOscillator;

  std::shared_ptr<Parameter> colorFolds;
  std::shared_ptr<WaveformOscillator> colorFoldsOscillator;

  std::shared_ptr<Parameter> grainAmount;
  std::shared_ptr<WaveformOscillator> grainAmountOscillator;

  std::shared_ptr<Parameter> colorShift;
  std::shared_ptr<WaveformOscillator> colorShiftOscillator;

  RisoColumnsSettings(std::string shaderId, json j) :
  speed(std::make_shared<Parameter>("Speed", 1.0, 0.0, 4.0)),
  speedOscillator(std::make_shared<WaveformOscillator>(speed)),
  columns(std::make_shared<Parameter>("Columns", 4.0, 1.0, 12.0)),
  columnsOscillator(std::make_shared<WaveformOscillator>(columns)),
  colorFolds(std::make_shared<Parameter>("Color Folds", 1.0, 1.0, 6.0)),
  colorFoldsOscillator(std::make_shared<WaveformOscillator>(colorFolds)),
  grainAmount(std::make_shared<Parameter>("Grain Amount", 0.06, 0.0, 0.50)),
  grainAmountOscillator(std::make_shared<WaveformOscillator>(grainAmount)),
  colorShift(std::make_shared<Parameter>("Color Shift", 0.0, 0.0, 1.0)),
  colorShiftOscillator(std::make_shared<WaveformOscillator>(colorShift)),
  ShaderSettings(shaderId, j, "RisoColumns") {
    parameters = { speed, columns, colorFolds, grainAmount, colorShift };
    oscillators = { speedOscillator, columnsOscillator, colorFoldsOscillator,
                    grainAmountOscillator, colorShiftOscillator };
    load(j);
    registerParameters();
  };
};

struct RisoColumnsShader: Shader {
  RisoColumnsSettings *settings;
  RisoColumnsShader(RisoColumnsSettings *settings) : settings(settings), Shader(settings) {};

  // Speed is integrated into an accumulated phase rather than multiplying
  // absolute time, so changing Speed alters the rate going forward instead of
  // instantly remapping (warping) the whole scene. See shade().
  float speedPhase = 0.0f;
  float lastTimeSample = 0.0f;
  bool timeSampled = false;

  void setup() override {
    shader.load("shaders/RisoColumns");
  }

  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override {
    // Integrate Speed into an accumulated phase: phase += dt * speed. Guards skip
    // the first frame and any large time gap (pause / scrub) so it never lurches.
    float now = TimeService::getService()->timeParam->value;
    float dt = timeSampled ? (now - lastTimeSample) : 0.0f;
    if (dt < 0.0f || dt > 0.25f) dt = 0.0f;
    lastTimeSample = now;
    timeSampled = true;
    speedPhase += dt * settings->speed->value;

    canvas->begin();
    shader.begin();
    shader.setUniformTexture("tex", frame->getTexture(), 4);
    shader.setUniform1f("time", speedPhase);
    shader.setUniform1f("columns", settings->columns->value);
    shader.setUniform1f("colorFolds", settings->colorFolds->value);
    shader.setUniform1f("grainAmount", settings->grainAmount->value);
    shader.setUniform1f("colorShift", settings->colorShift->value);
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
    return ShaderTypeRisoColumns;
  }

  void drawSettings() override {
    CommonViews::H3Title("RisoColumns");

    CommonViews::ShaderParameter(settings->speed, settings->speedOscillator);
    CommonViews::ShaderParameter(settings->columns, settings->columnsOscillator);
    CommonViews::ShaderParameter(settings->colorFolds, settings->colorFoldsOscillator);
    CommonViews::ShaderParameter(settings->grainAmount, settings->grainAmountOscillator);
    CommonViews::ShaderParameter(settings->colorShift, settings->colorShiftOscillator);
  }
};

#endif /* RisoColumnsShader_hpp */
