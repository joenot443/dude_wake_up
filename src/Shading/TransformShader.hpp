//
//  TransformShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef TransformShader_hpp
#define TransformShader_hpp

#include "ofMain.h"

#include "ShaderConfigSelectionView.hpp"
#include "ImGuiExtensions.hpp"
#include "Shader.hpp"
#include "ofxImGui.h"
#include <stdio.h>

// Transform

struct TransformSettings : public ShaderSettings {
	public:
  std::shared_ptr<Parameter> xTranslate;
  std::shared_ptr<Parameter> yTranslate;
  std::shared_ptr<Parameter> xScale;
  std::shared_ptr<Parameter> yScale;
  std::shared_ptr<Parameter> rotate;

  std::shared_ptr<Parameter> autoRotate;
  std::shared_ptr<Parameter> lockXYScale;

  std::shared_ptr<Oscillator> xTranslateOscillator;
  std::shared_ptr<Oscillator> yTranslateOscillator;
  std::shared_ptr<Oscillator> xScaleOscillator;
  std::shared_ptr<Oscillator> yScaleOscillator;
  std::shared_ptr<Oscillator> rotateOscillator;
  std::shared_ptr<Oscillator> autoRotateOscillator;

  TransformSettings(std::string shaderId, json j)
      : xTranslate(std::make_shared<Parameter>("xTranslate", 0.0,
                                               -1.0, 1.0)),
        yTranslate(std::make_shared<Parameter>("yTranslate", 0.0,
                                               -1.0, 1.0)),
        xScale(std::make_shared<Parameter>("xScale", 1.0, 0.0, 10.0)),
        yScale(std::make_shared<Parameter>("yScale", 1.0, 0.0, 10.0)),
        rotate(
            std::make_shared<Parameter>("rotate", 0.0, 0.0, 360.0)),
        autoRotate(std::make_shared<Parameter>("autoRotate", 0.0, 0.0,
                                               2.0)),
  lockXYScale(std::make_shared<Parameter>("Lock XY Scale", 1.0, 0.0,
                                         1.0)),
        xTranslateOscillator(std::make_shared<WaveformOscillator>(xTranslate)),
        yTranslateOscillator(std::make_shared<WaveformOscillator>(yTranslate)),
        xScaleOscillator(std::make_shared<WaveformOscillator>(xScale)),
        yScaleOscillator(std::make_shared<WaveformOscillator>(yScale)),
        rotateOscillator(std::make_shared<WaveformOscillator>(rotate)),
        autoRotateOscillator(std::make_shared<WaveformOscillator>(autoRotate)),
        ShaderSettings(shaderId, j) {
    parameters = {xTranslate, yTranslate, xScale, yScale, rotate, autoRotate, lockXYScale};
    oscillators = {xTranslateOscillator, yTranslateOscillator,
                   xScaleOscillator,     yScaleOscillator,
                   rotateOscillator,     autoRotateOscillator};
    load(j);
  registerParameters();
  }
};

struct TransformShader : public Shader {
public:
  ofShader shader;
  TransformSettings *settings;
  float autoRotateAmount;

  TransformShader(TransformSettings *settings)
      : settings(settings), Shader(settings){};

  void setup() override {}

  ShaderType type() override { return ShaderTypeTransform; }

  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override {
    canvas->begin();
    auto texture = frame->getTexture();
    float width = frame->getWidth();
    float height = frame->getHeight();
    bool autoRotating = false;

    // Increment autoRotate
    if (settings->autoRotate->value > 0.01) {
      autoRotateAmount += settings->autoRotate->value;
      autoRotating = true;
    }

    ofClear(0, 0, 0, 0);

    ofSetRectMode(OF_RECTMODE_CENTER);
    ofPushMatrix();

    float x = (settings->xTranslate->value + 0.5) * width;
    float y = (settings->yTranslate->value + 0.5) * height;
    
    ofTranslate(x, y);
    if (autoRotating) {
      ofRotateDeg(autoRotateAmount);
    } else {
      ofRotateDeg(settings->rotate->value);
    }

    if (settings->lockXYScale->boolValue) {
      ofScale(settings->xScale->value, settings->xScale->value);
    } else {
      ofScale(settings->xScale->value, settings->yScale->value);
    }
    
    texture.draw(0, 0);
    ofPopMatrix();

    shader.end();
    canvas->end();
  }

  void drawSettings() override {
    
    ImGui::Columns(2);
    ImGui::SetColumnWidth(0, 200);
    ImGuiExtensions::Slider2DFloat("", &settings->xTranslate->value, &settings->yTranslate->value, -1., 1.0, -1., 1.0, 1.0);
    ImGui::NextColumn();
    ImGui::Text("Oscillate X");
    ImGui::SameLine();
    CommonViews::OscillateButton("##yOscillator", settings->xTranslateOscillator, settings->xTranslate);
    ImGui::Text("Oscillate Y");
    ImGui::SameLine();
    CommonViews::OscillateButton("##yOscillator", settings->yTranslateOscillator, settings->yTranslate);
    ImGui::Columns(1);
    CommonViews::ShaderCheckbox(settings->lockXYScale);

    // Scale X
    CommonViews::ShaderParameter(settings->xScale, settings->xScaleOscillator);

    if (!settings->lockXYScale->boolValue) {
      // Scale Y
      CommonViews::ShaderParameter(settings->yScale, settings->yScaleOscillator);
    }
    // Auto Rotate
    CommonViews::ShaderParameter(settings->autoRotate,
                                 settings->autoRotateOscillator);

    // Rotate
    CommonViews::ShaderParameter(settings->rotate, settings->rotateOscillator);
  }

  void clear() override {}
};

#endif
