//
//  SolarWaveShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef SolarWaveShader_hpp
#define SolarWaveShader_hpp

#include "ofMain.h"
#include "ShaderSettings.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "TimeService.hpp"
#include "WaveformOscillator.hpp"
#include "Parameter.hpp"
#include "Shader.hpp"
#include <stdio.h>

struct SolarWaveSettings: public ShaderSettings {
  std::shared_ptr<Parameter> frequency;
  std::shared_ptr<WaveformOscillator> frequencyOscillator;

  std::shared_ptr<Parameter> phase;
  std::shared_ptr<WaveformOscillator> phaseOscillator;

  std::shared_ptr<Parameter> phaseSpread;
  std::shared_ptr<WaveformOscillator> phaseSpreadOscillator;

  std::shared_ptr<Parameter> drive;
  std::shared_ptr<WaveformOscillator> driveOscillator;

  std::shared_ptr<Parameter> bias;
  std::shared_ptr<WaveformOscillator> biasOscillator;

  std::shared_ptr<Parameter> mixAmount;
  std::shared_ptr<WaveformOscillator> mixAmountOscillator;

  SolarWaveSettings(std::string shaderId, json j) :
  frequency(std::make_shared<Parameter>("Frequency", 1.0, 0.5, 6.0)),
  frequencyOscillator(std::make_shared<WaveformOscillator>(frequency)),
  phase(std::make_shared<Parameter>("Phase", 0.0, 0.0, 6.2832)),
  phaseOscillator(std::make_shared<WaveformOscillator>(phase)),
  phaseSpread(std::make_shared<Parameter>("Phase Spread", 0.0, -3.1416, 3.1416)),
  phaseSpreadOscillator(std::make_shared<WaveformOscillator>(phaseSpread)),
  drive(std::make_shared<Parameter>("Drive", 1.0, 0.5, 3.0)),
  driveOscillator(std::make_shared<WaveformOscillator>(drive)),
  bias(std::make_shared<Parameter>("Bias", 0.0, -0.5, 0.5)),
  biasOscillator(std::make_shared<WaveformOscillator>(bias)),
  mixAmount(std::make_shared<Parameter>("Mix", 1.0, 0.0, 1.0)),
  mixAmountOscillator(std::make_shared<WaveformOscillator>(mixAmount)),
  ShaderSettings(shaderId, j, "SolarWave") {
    parameters = { frequency, phase, phaseSpread, drive, bias, mixAmount };
    oscillators = { frequencyOscillator, phaseOscillator, phaseSpreadOscillator, driveOscillator, biasOscillator, mixAmountOscillator };
    load(j);
    registerParameters();
  };
};

struct SolarWaveShader: Shader {
  SolarWaveSettings *settings;
  SolarWaveShader(SolarWaveSettings *settings) : settings(settings), Shader(settings) {};

  void setup() override {
    shader.load("shaders/SolarWave");
  }

  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override {
    canvas->begin();
    shader.begin();
    shader.setUniformTexture("tex", frame->getTexture(), 4);
    shader.setUniform1f("frequency", settings->frequency->value);
    shader.setUniform1f("phase", settings->phase->value);
    shader.setUniform1f("phaseSpread", settings->phaseSpread->value);
    shader.setUniform1f("drive", settings->drive->value);
    shader.setUniform1f("bias", settings->bias->value);
    shader.setUniform1f("mixAmount", settings->mixAmount->value);
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
    return ShaderTypeSolarWave;
  }

  void drawSettings() override {
    CommonViews::H3Title("SolarWave");

    CommonViews::ShaderParameter(settings->frequency, settings->frequencyOscillator);
    CommonViews::ShaderParameter(settings->phase, settings->phaseOscillator);
    CommonViews::ShaderParameter(settings->phaseSpread, settings->phaseSpreadOscillator);
    CommonViews::ShaderParameter(settings->drive, settings->driveOscillator);
    CommonViews::ShaderParameter(settings->bias, settings->biasOscillator);
    CommonViews::ShaderParameter(settings->mixAmount, settings->mixAmountOscillator);
  }
};

#endif /* SolarWaveShader_hpp */
