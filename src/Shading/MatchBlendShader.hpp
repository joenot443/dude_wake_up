//
//  MatchBlendShader.hpp
//  dude_wake_up
//
//  Blends two sources where the per-pixel mix strength is driven by how
//  closely the two sources "match" at that pixel. Closer match -> stronger
//  blend. The match scalar is computed from a selectable metric.
//

#ifndef MatchBlendShader_hpp
#define MatchBlendShader_hpp

#include "ofMain.h"
#include "ShaderSettings.hpp"
#include "TimeService.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "Shader.hpp"
#include <stdio.h>

static std::vector<std::string> MatchBlendMetricNames = {
  "RGB Distance",
  "Luminance",
  "Hue",
  "Saturation"
};

struct MatchBlendSettings : public ShaderSettings
{
  std::shared_ptr<Parameter> metric;
  std::shared_ptr<Parameter> threshold;
  std::shared_ptr<Parameter> smoothness;
  std::shared_ptr<Parameter> strength;
  std::shared_ptr<Parameter> invert;
  std::shared_ptr<Parameter> swap;
  std::shared_ptr<Parameter> showMask;

  std::shared_ptr<WaveformOscillator> thresholdOscillator;
  std::shared_ptr<WaveformOscillator> smoothnessOscillator;
  std::shared_ptr<WaveformOscillator> strengthOscillator;

  MatchBlendSettings(std::string shaderId, json j, std::string name)
    : metric(std::make_shared<Parameter>("Metric", 0.0, 0.0, 3.0, ParameterType_Int)),
      threshold(std::make_shared<Parameter>("Threshold", 0.25, 0.0, 1.0)),
      smoothness(std::make_shared<Parameter>("Smoothness", 0.5, 0.0, 1.0)),
      strength(std::make_shared<Parameter>("Strength", 1.0, 0.0, 1.0)),
      invert(std::make_shared<Parameter>("Invert", false, ParameterType_Bool)),
      swap(std::make_shared<Parameter>("Swap", false, ParameterType_Bool)),
      showMask(std::make_shared<Parameter>("Show Mask", false, ParameterType_Bool)),
      thresholdOscillator(std::make_shared<WaveformOscillator>(threshold)),
      smoothnessOscillator(std::make_shared<WaveformOscillator>(smoothness)),
      strengthOscillator(std::make_shared<WaveformOscillator>(strength)),
      ShaderSettings(shaderId, j, name)
  {
    metric->options = MatchBlendMetricNames;
    metric->buttonsVisible = true;

    parameters = {metric, threshold, smoothness, strength, invert, swap, showMask};
    oscillators = {thresholdOscillator, smoothnessOscillator, strengthOscillator};
    load(j);
    registerParameters();
  };
};

struct MatchBlendShader : Shader
{
  MatchBlendSettings *settings;
  MatchBlendShader(MatchBlendSettings *settings) : settings(settings), Shader(settings){};

  void setup() override
  {
    shader.load("shaders/MatchBlend");
  }

  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override
  {
    canvas->begin();
    shader.begin();
    ofClear(0, 0, 0, 255);
    ofClear(0, 0, 0, 0);

    // Source A is the main input (the incoming frame).
    ofTexture texA = frame->getTexture();

    // Source B is the second input; fall back to A if nothing connected so
    // the node still renders (match is then uniformly perfect).
    ofTexture texB = texA;
    if (hasInputAtSlot(InputSlotTwo))
    {
      auto secondFrame = inputAtSlot(InputSlotTwo)->frame();
      if (secondFrame && secondFrame->isAllocated())
      {
        texB = secondFrame->getTexture();
      }
    }

    if (settings->swap->boolValue)
    {
      shader.setUniformTexture("tex", texB, 4);
      shader.setUniformTexture("tex2", texA, 5);
    }
    else
    {
      shader.setUniformTexture("tex", texA, 4);
      shader.setUniformTexture("tex2", texB, 5);
    }

    shader.setUniform1i("metric", settings->metric->intValue);
    shader.setUniform1f("threshold", settings->threshold->value);
    shader.setUniform1f("smoothness", settings->smoothness->value);
    shader.setUniform1f("strength", settings->strength->value);
    shader.setUniform1i("invert", settings->invert->boolValue ? 1 : 0);
    shader.setUniform1i("showMask", settings->showMask->boolValue ? 1 : 0);
    shader.setUniform1f("time", TimeService::getService()->timeParam->value);
    shader.setUniform2f("dimensions", frame->getWidth(), frame->getHeight());

    frame->draw(0, 0);
    shader.end();
    canvas->end();
  }

  void clear() override
  {
  }

  int inputCount() override
  {
    return 2;
  }

  ShaderType type() override
  {
    return ShaderTypeMatchBlend;
  }

  void drawSettings() override
  {
    CommonViews::ShaderOption(settings->metric, MatchBlendMetricNames);
    CommonViews::ShaderParameter(settings->threshold, settings->thresholdOscillator);
    CommonViews::ShaderParameter(settings->smoothness, settings->smoothnessOscillator);
    CommonViews::ShaderParameter(settings->strength, settings->strengthOscillator);
    CommonViews::ShaderCheckbox(settings->invert);
    CommonViews::ShaderCheckbox(settings->swap);
    CommonViews::ShaderCheckbox(settings->showMask);
  }
};

#endif
