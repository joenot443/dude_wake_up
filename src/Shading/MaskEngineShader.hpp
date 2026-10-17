//
//  MaskEngineShader.hpp
//  dude_wake_up
//
//  Unified mask/blend shader. One node with three selectable presets
//  (Luma, Color Key, Match Blend) sharing a soft-falloff mask pipeline.
//  Subsumes LumaMaskMaker, ColorKeyMaskMaker and MatchBlend.
//

#ifndef MaskEngineShader_hpp
#define MaskEngineShader_hpp

#include "ofMain.h"
#include "ShaderSettings.hpp"
#include "TimeService.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "Shader.hpp"
#include <stdio.h>

// Source modes (presets)
enum MaskSourceMode {
  MaskSourceLumaRange = 0,
  MaskSourceColorKey = 1,
  MaskSourceMatch = 2,
};

static std::vector<std::string> MaskPresetNames = {"Luma", "Color Key", "Match Blend"};
static std::vector<std::string> MaskOutputNames = {"Mask", "Blend"};
static std::vector<std::string> MaskLumaChannelNames = {"Brightness", "Saturation", "Red", "Green", "Blue"};
static std::vector<std::string> MaskColorMetricNames = {"YUV Chroma", "RGB", "Hue", "Saturation"};
static std::vector<std::string> MaskMatchMetricNames = {"RGB Distance", "Luminance", "Hue", "Saturation"};
static std::vector<std::string> MaskRegionNames = {"Input A", "Input B", "Transparent"};

struct MaskEngineSettings : public ShaderSettings {
  std::shared_ptr<Parameter> sourceMode;
  std::shared_ptr<Parameter> outputMode;
  std::shared_ptr<Parameter> metric;

  std::shared_ptr<Parameter> upper;
  std::shared_ptr<Parameter> lower;
  std::shared_ptr<Parameter> color;
  std::shared_ptr<Parameter> tolerance;
  std::shared_ptr<Parameter> threshold;

  std::shared_ptr<Parameter> softness;
  std::shared_ptr<Parameter> strength;
  std::shared_ptr<Parameter> invert;
  std::shared_ptr<Parameter> swap;
  std::shared_ptr<Parameter> showMask;

  std::shared_ptr<Parameter> matchedSource;
  std::shared_ptr<Parameter> unmatchedSource;

  std::shared_ptr<WaveformOscillator> upperOscillator;
  std::shared_ptr<WaveformOscillator> lowerOscillator;
  std::shared_ptr<WaveformOscillator> toleranceOscillator;
  std::shared_ptr<WaveformOscillator> thresholdOscillator;
  std::shared_ptr<WaveformOscillator> softnessOscillator;
  std::shared_ptr<WaveformOscillator> strengthOscillator;

  MaskEngineSettings(std::string shaderId, json j, std::string name)
    : sourceMode(std::make_shared<Parameter>("Source Mode", 0.0, 0.0, 2.0, ParameterType_Int)),
      outputMode(std::make_shared<Parameter>("Output Mode", 0.0, 0.0, 1.0, ParameterType_Int)),
      metric(std::make_shared<Parameter>("Metric", 0.0, 0.0, 4.0, ParameterType_Int)),
      upper(std::make_shared<Parameter>("Upper", 1.0, 0.0, 1.0)),
      lower(std::make_shared<Parameter>("Lower", 0.5, 0.0, 1.0)),
      color(std::make_shared<Parameter>("Color", 0.0, ParameterType_Color)),
      tolerance(std::make_shared<Parameter>("Tolerance", 0.1, 0.0, 1.0)),
      threshold(std::make_shared<Parameter>("Threshold", 0.25, 0.0, 1.0)),
      softness(std::make_shared<Parameter>("Softness", 0.0, 0.0, 1.0)),
      strength(std::make_shared<Parameter>("Strength", 1.0, 0.0, 1.0)),
      invert(std::make_shared<Parameter>("Invert", false, ParameterType_Bool)),
      swap(std::make_shared<Parameter>("Swap", false, ParameterType_Bool)),
      showMask(std::make_shared<Parameter>("Show Mask", false, ParameterType_Bool)),
      matchedSource(std::make_shared<Parameter>("Matched Source", 0.0, 0.0, 2.0, ParameterType_Int)),
      unmatchedSource(std::make_shared<Parameter>("Unmatched Source", 2.0, 0.0, 2.0, ParameterType_Int)),
      upperOscillator(std::make_shared<WaveformOscillator>(upper)),
      lowerOscillator(std::make_shared<WaveformOscillator>(lower)),
      toleranceOscillator(std::make_shared<WaveformOscillator>(tolerance)),
      thresholdOscillator(std::make_shared<WaveformOscillator>(threshold)),
      softnessOscillator(std::make_shared<WaveformOscillator>(softness)),
      strengthOscillator(std::make_shared<WaveformOscillator>(strength)),
      ShaderSettings(shaderId, j, name)
  {
    // Named options make these render as dropdowns (DSPicker in NottawaApp,
    // combo in ImGui) instead of int steppers. Metric's list is mode-dependent
    // and refreshed each frame in shade().
    sourceMode->options = MaskPresetNames;
    outputMode->options = MaskOutputNames;
    metric->options = MaskMatchMetricNames;
    matchedSource->options = MaskRegionNames;
    unmatchedSource->options = MaskRegionNames;

    parameters = {sourceMode, outputMode, metric, upper, lower, color, tolerance,
                  threshold, softness, strength, invert, swap, showMask,
                  matchedSource, unmatchedSource};
    oscillators = {upperOscillator, lowerOscillator, toleranceOscillator,
                   thresholdOscillator, softnessOscillator, strengthOscillator};
    load(j);
    registerParameters();
  };
};

struct MaskEngineShader : Shader {
  MaskEngineSettings *settings;
  MaskEngineShader(MaskEngineSettings *settings) : settings(settings), Shader(settings){};

  void setup() override {
    shader.load("shaders/MaskEngine");
  }

  // Apply a preset: set the source mode and a sensible default output mode.
  void applyPreset(int mode) {
    settings->sourceMode->intValue = mode;
    settings->sourceMode->affirmIntValue();
    settings->outputMode->intValue = (mode == MaskSourceMatch) ? 1 : 0;
    settings->outputMode->affirmIntValue();
    settings->metric->intValue = 0;
    settings->metric->affirmIntValue();
  }

  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override {
    // Keep the Metric dropdown's labels in sync with the selected source mode
    // (the inspector reads options live from the parameter).
    switch (settings->sourceMode->intValue) {
      case MaskSourceLumaRange: settings->metric->options = MaskLumaChannelNames; break;
      case MaskSourceColorKey:  settings->metric->options = MaskColorMetricNames; break;
      default:                  settings->metric->options = MaskMatchMetricNames; break;
    }

    // Mode-based parameter visibility. hiddenFromNode propagates to both the
    // ImGui node and the NottawaApp inspector list, so this is the single
    // source of truth for which controls apply to the current preset.
    bool luma  = settings->sourceMode->intValue == MaskSourceLumaRange;
    bool color = settings->sourceMode->intValue == MaskSourceColorKey;
    bool match = settings->sourceMode->intValue == MaskSourceMatch;
    bool maskOut = settings->outputMode->intValue == 0;

    settings->sourceMode->hiddenFromNode = true; // driven by the preset pane
    settings->upper->hiddenFromNode = !luma;
    settings->lower->hiddenFromNode = !luma;
    settings->color->hiddenFromNode = !color;
    settings->tolerance->hiddenFromNode = !color;
    settings->threshold->hiddenFromNode = !match;
    settings->swap->hiddenFromNode = !match;
    settings->matchedSource->hiddenFromNode = !maskOut;
    settings->unmatchedSource->hiddenFromNode = !maskOut;

    canvas->begin();
    shader.begin();
    ofClear(0, 0, 0, 255);
    ofClear(0, 0, 0, 0);

    ofTexture texA = frame->getTexture();
    ofTexture texB = texA;
    if (hasInputAtSlot(InputSlotTwo)) {
      auto secondFrame = inputAtSlot(InputSlotTwo)->frame();
      if (secondFrame && secondFrame->isAllocated()) {
        texB = secondFrame->getTexture();
      }
    }

    shader.setUniformTexture("tex", texA, 4);
    shader.setUniformTexture("tex2", texB, 5);

    shader.setUniform1i("sourceMode", settings->sourceMode->intValue);
    shader.setUniform1i("outputMode", settings->outputMode->intValue);
    shader.setUniform1i("metric", settings->metric->intValue);
    shader.setUniform1f("lower", settings->lower->value);
    shader.setUniform1f("upper", settings->upper->value);
    shader.setUniform1f("tolerance", settings->tolerance->value);
    shader.setUniform1f("threshold", settings->threshold->value);
    shader.setUniform1f("softness", settings->softness->value);
    shader.setUniform1f("strength", settings->strength->value);
    shader.setUniform1i("invert", settings->invert->boolValue ? 1 : 0);
    shader.setUniform1i("swapInputs", settings->swap->boolValue ? 1 : 0);
    shader.setUniform1i("showMask", settings->showMask->boolValue ? 1 : 0);
    shader.setUniform1i("matchedSource", settings->matchedSource->intValue);
    shader.setUniform1i("unmatchedSource", settings->unmatchedSource->intValue);
    shader.setUniform4f("keyColor", settings->color->color->data()[0],
                        settings->color->color->data()[1],
                        settings->color->color->data()[2], 1.0);
    shader.setUniform1f("time", TimeService::getService()->timeParam->value);
    shader.setUniform2f("dimensions", frame->getWidth(), frame->getHeight());

    frame->draw(0, 0);
    shader.end();
    canvas->end();
  }

  void clear() override {}

  int inputCount() override {
    return 2;
  }

  ShaderType type() override {
    return ShaderTypeMaskEngine;
  }

  void drawSettings() override {
    int mode = settings->sourceMode->intValue;

    // --- Preset pane: three cards selecting the source mode ---
    CommonViews::H3Title("Preset");
    float paneWidth = ImGui::GetContentRegionAvail().x;
    float buttonWidth = (paneWidth - 2.0 * ImGui::GetStyle().ItemSpacing.x) / 3.0;
    for (int i = 0; i < (int)MaskPresetNames.size(); i++) {
      bool active = (mode == i);
      if (active) {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.20, 0.50, 0.90, 1.0));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.26, 0.56, 0.96, 1.0));
      }
      if (ImGui::Button(MaskPresetNames[i].c_str(), ImVec2(buttonWidth, 30.0))) {
        applyPreset(i);
        mode = i;
      }
      if (active) {
        ImGui::PopStyleColor(2);
      }
      if (i < (int)MaskPresetNames.size() - 1) ImGui::SameLine();
    }

    ImGui::Dummy(ImVec2(1.0, 6.0));

    // Output mode (independently toggleable for cross-mode combinations)
    CommonViews::Selector(settings->outputMode, MaskOutputNames);

    ImGui::Dummy(ImVec2(1.0, 6.0));

    // --- Mode-specific controls ---
    if (mode == MaskSourceLumaRange) {
      settings->metric->options = MaskLumaChannelNames;
      CommonViews::Selector(settings->metric, MaskLumaChannelNames);
      CommonViews::RangeSlider("Luma Range", settings->upper->paramId, settings->upper,
                               settings->lower,
                               formatString("%.2f - %.2f", settings->lower->value, settings->upper->value));
    } else if (mode == MaskSourceColorKey) {
      settings->metric->options = MaskColorMetricNames;
      CommonViews::Selector(settings->metric, MaskColorMetricNames);
      CommonViews::ShaderColor(settings->color);
      CommonViews::ShaderParameter(settings->tolerance, settings->toleranceOscillator);
    } else {
      settings->metric->options = MaskMatchMetricNames;
      CommonViews::Selector(settings->metric, MaskMatchMetricNames);
      CommonViews::ShaderParameter(settings->threshold, settings->thresholdOscillator);
      CommonViews::ShaderCheckbox(settings->swap);
    }

    ImGui::Dummy(ImVec2(1.0, 6.0));

    // --- Shared controls ---
    CommonViews::ShaderParameter(settings->softness, settings->softnessOscillator);
    CommonViews::ShaderParameter(settings->strength, settings->strengthOscillator);
    CommonViews::ShaderCheckbox(settings->invert);
    CommonViews::ShaderCheckbox(settings->showMask);

    // --- Region pickers (Mask output only) ---
    if (settings->outputMode->intValue == 0) {
      ImGui::Dummy(ImVec2(1.0, 6.0));
      CommonViews::Selector(settings->matchedSource, MaskRegionNames);
      CommonViews::Selector(settings->unmatchedSource, MaskRegionNames);
    }
  }
};

#endif
