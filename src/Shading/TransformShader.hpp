#ifndef TransformShader_hpp
#define TransformShader_hpp

#include "ofMain.h"
#include "ImHelpers.h"
#include "Fonts.hpp"
#include "ShaderConfigSelectionView.hpp"
#include "ImGuiExtensions.hpp"
#include "CommonViews.hpp"
#include "Shader.hpp"
#include "SourceTransform.hpp"
#include "ofxImGui.h"
#include <stdio.h>

// Transform — wraps a SourceTransform helper that handles the two-pass
// crop + place pipeline. The same helper is used by sources (WebcamSource,
// etc.) so the inspector UX is identical across all of them.

struct TransformSettings : public ShaderSettings {
public:
  SourceTransform transform;

  TransformSettings(std::string shaderId, json j, std::string name)
  : transform(shaderId), ShaderSettings(shaderId, j, name) {
    transform.appendParameters(parameters);
    transform.appendOscillators(oscillators);
    load(j);
    audioReactiveParameter = transform.scale;
    registerParameters();
  }
};

struct TransformShader : public Shader {
public:
  TransformSettings *settings;

  TransformShader(TransformSettings *settings)
  : settings(settings), Shader(settings) {};

  void setup() override {}

  int inputCount() override {
    return 1;
  }

  ShaderType type() override { return ShaderTypeTransform; }

  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override {
    auto texture = frame->getTexture();
    settings->transform.apply(texture, canvas);
  }

  void drawSettings() override {
    auto frame = isOptional ? lastFrame : parentFrame();
    auto cursorPos = ImGui::GetCursorPos();
    if (frame != nullptr && frame->isAllocated()) {
      ofxImGui::AddImage(frame->getTexture(), ofVec2f(256.0, 144.0));
    }
    ImGui::SetCursorPos(cursorPos);

    auto& t = settings->transform;

    if (CommonViews::AreaSlider(shaderId, t.minX, t.maxX, t.minY, t.maxY,
                                 t.minXOscillator, t.maxXOscillator,
                                 t.minYOscillator, t.maxYOscillator)) {
      t.translateY->setValue(t.minY->value);
      t.translateX->setValue(t.minX->value);
    }

    // X
    auto pos = ImGui::GetCursorPos();
    CommonViews::MiniSlider(t.minX);
    ImGui::SetCursorPos(ImVec2(pos.x + 120.0, pos.y));
    CommonViews::MiniSlider(t.maxX);
    ImGui::SameLine();
    if (CommonViews::ResetButton(shaderId, t.minX)) {
      t.maxX->resetValue();
      t.minY->resetValue();
      t.maxY->resetValue();
    }

    // Y
    pos = ImGui::GetCursorPos();
    CommonViews::MiniSlider(t.minY);
    ImGui::SetCursorPos(ImVec2(pos.x + 120.0, pos.y));
    CommonViews::MiniSlider(t.maxY);

    CommonViews::ShaderParameter(t.scale, t.scaleOscillator);
    CommonViews::ShaderCheckbox(t.scaleFromCenter);

    CommonViews::MultiSlider("Position", shaderId, t.translateX, t.translateY,
                              t.translateXOscillator, t.translateYOscillator, 0.5625);

    CommonViews::ShaderCheckbox(t.verticalFlip);
    CommonViews::ShaderCheckbox(t.horizontalFlip);

    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 5.0);
    CommonViews::ShaderCheckbox(t.center);
    CommonViews::Selector(t.mode, t.mode->options);

    CommonViews::ShaderCheckbox(t.backgroundEnabled);
    if (t.backgroundEnabled->boolValue) {
      CommonViews::ShaderColor(t.backgroundColor);
    }
  }

  void clear() override {}
};

#endif
