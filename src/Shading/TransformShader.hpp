#ifndef TransformShader_hpp
#define TransformShader_hpp

#include "ofMain.h"
#include "ImHelpers.h"
#include "Fonts.hpp"
#include "ShaderConfigSelectionView.hpp"
#include "ImGuiExtensions.hpp"
#include "CommonViews.hpp"
#include "Shader.hpp"
#include "ofxImGui.h"
#include <stdio.h>

// Transform

struct TransformSettings : public ShaderSettings {
public:
  std::shared_ptr<Parameter> minX;
  std::shared_ptr<Parameter> maxX;
  std::shared_ptr<Parameter> minY;
  std::shared_ptr<Parameter> maxY;
  std::shared_ptr<Parameter> rotate;
  
  std::shared_ptr<Parameter> translateX;
  std::shared_ptr<Parameter> translateY;
  std::shared_ptr<Parameter> center;

  std::shared_ptr<Parameter> verticalFlip;
  std::shared_ptr<Parameter> horizontalFlip;
  
  std::shared_ptr<Parameter> scale;
  std::shared_ptr<Parameter> mode;
  std::shared_ptr<WaveformOscillator> minXOscillator;
  std::shared_ptr<WaveformOscillator> maxXOscillator;
  std::shared_ptr<WaveformOscillator> minYOscillator;
  std::shared_ptr<WaveformOscillator> maxYOscillator;
  std::shared_ptr<WaveformOscillator> translateXOscillator;
  std::shared_ptr<WaveformOscillator> translateYOscillator;
  std::shared_ptr<WaveformOscillator> scaleOscillator;

  
  TransformSettings(std::string shaderId, json j, std::string name)
  : minX(std::make_shared<Parameter>("Min X", 0.0, 0.0, 1.0)),
  maxX(std::make_shared<Parameter>("Max X", 1.0, 0.0, 1.0)),
  minY(std::make_shared<Parameter>("Min Y", 0.0, 0.0, 1.0)),
  maxY(std::make_shared<Parameter>("Max Y", 1.0, 0.0, 1.0)),
  translateX(std::make_shared<Parameter>("X", 0.0, 0.0, 1.0)),
  translateY(std::make_shared<Parameter>("Y", 0.0, 0.0, 1.0)),
  verticalFlip(std::make_shared<Parameter>("Vertical Flip", 0.0, 0.0, 1.0, ParameterType_Bool)),
  horizontalFlip(std::make_shared<Parameter>("Horizontal Flip", 0.0, 0.0, 1.0, ParameterType_Bool)),
  scale(std::make_shared<Parameter>("Scale", 1.0, 0.0, 5.0)),
  mode(std::make_shared<Parameter>("Mode", 0, ParameterType_Int)),
  center(std::make_shared<Parameter>("Center", ParameterType_Bool)),
  minXOscillator(std::make_shared<WaveformOscillator>(minX)),
  maxXOscillator(std::make_shared<WaveformOscillator>(maxX)),
  minYOscillator(std::make_shared<WaveformOscillator>(minY)),
  maxYOscillator(std::make_shared<WaveformOscillator>(maxY)),
  translateXOscillator(std::make_shared<WaveformOscillator>(translateX)),
  translateYOscillator(std::make_shared<WaveformOscillator>(translateY)),
  scaleOscillator(std::make_shared<WaveformOscillator>(scale)),
  ShaderSettings(shaderId, j, name) {
    mode->options = {"Standard", "Fill", "Fit", "Fill + Aspect"};
    parameters = {minX, maxX, minY, maxY, scale, translateX, translateY, mode, center, verticalFlip, horizontalFlip};
    oscillators = {minXOscillator, maxXOscillator, minYOscillator, maxYOscillator, translateXOscillator, translateYOscillator, scaleOscillator};
    load(j);
    audioReactiveParameter = scale;
    registerParameters();
  }
};

struct TransformShader : public Shader {
public:

  TransformSettings *settings;
  float autoRotateAmount;
  
  TransformShader(TransformSettings *settings)
  : settings(settings), Shader(settings){};
  
  void setup() override {}
  
  int inputCount() override {
    return 1;
  }
  
  ShaderType type() override { return ShaderTypeTransform; }
  
  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override {
    canvas->begin();
    
    auto texture = frame->getTexture();
    float frameWidth = frame->getWidth();
    float frameHeight = frame->getHeight();
    
    // Define cropping region based on parameters
    float cropX = settings->minX->value * frameWidth;
    float cropY = settings->minY->value * frameHeight;
    float cropWidth = (settings->maxX->value - settings->minX->value) * frameWidth;
    float cropHeight = (settings->maxY->value - settings->minY->value) * frameHeight;
    float drawWidth = cropWidth * settings->scale->value;
    float drawHeight = cropHeight * settings->scale->value;
    float translateX = 0;
    float translateY = 0;
    
    // Calculate scaling based on mode
    float canvasAspect = canvas->getWidth() / (float)canvas->getHeight();
    float cropAspect = cropWidth / cropHeight;
    
    switch(settings->mode->intValue) {
      case 1: // Fill (stretch to fill)
        break;
      case 2: // Fit (fit within bounds)
        break;
      case 3: // Fill + Aspect (fill while maintaining aspect ratio)
        break;
      default: // Standard (manual scaling)
        translateX = settings->center->boolValue ? (canvas->getWidth() - cropWidth) / 2 : settings->translateX->value * canvas->getWidth();
        translateY = settings->center->boolValue ? (canvas->getHeight() - cropHeight) / 2 : settings->translateY->value * canvas->getHeight();
        break;
    }

    if (settings->verticalFlip->value) {
      ofScale(1, -1);
    }

    if (settings->horizontalFlip->value) {
      ofScale(-1, 1);
    }
    
    ofClear(0, 0, 0, 0);
    ofPushMatrix();
    // Draw the cropped and scaled texture
    texture.drawSubsection(translateX, translateY,
                           settings->scale->value * cropWidth,
                           settings->scale->value * cropHeight,
                           cropX, cropY,
                           cropWidth, cropHeight);
    
    ofPopMatrix();
    
    shader.end();
    canvas->end();
  }
  
  
  void drawSettings() override {
    auto frame = isOptional ? lastFrame : parentFrame();
    auto cursorPos = ImGui::GetCursorPos();
    if (frame != nullptr && frame->isAllocated()) {
      ofxImGui::AddImage(frame->getTexture(), ofVec2f(256.0, 144.0));
    }
    ImGui::SetCursorPos(cursorPos);
    
    if (CommonViews::AreaSlider(shaderId, settings->minX, settings->maxX, settings->minY, settings->maxY, settings->minXOscillator, settings->maxXOscillator, settings->minYOscillator, settings->maxYOscillator)) {
      settings->translateY->setValue(settings->minY->value);
      settings->translateX->setValue(settings->minX->value);
    }
    
    // X
    auto pos = ImGui::GetCursorPos();
    CommonViews::MiniSlider(settings->minX);
    ImGui::SetCursorPos(ImVec2(pos.x + 120.0, pos.y));
    CommonViews::MiniSlider(settings->maxX);
    ImGui::SameLine();
    if (CommonViews::ResetButton(shaderId, settings->minX)) {
      settings->maxX->resetValue();
      settings->minY->resetValue();
      settings->maxY->resetValue();
    }
    
    // Y
    pos = ImGui::GetCursorPos();
    CommonViews::MiniSlider(settings->minY);
    ImGui::SetCursorPos(ImVec2(pos.x + 120.0, pos.y));
    CommonViews::MiniSlider(settings->maxY);
    
    CommonViews::ShaderParameter(settings->scale, settings->scaleOscillator);
    
    CommonViews::MultiSlider("Position", settings->shaderId, settings->translateX, settings->translateY, settings->translateXOscillator, settings->translateYOscillator, 0.5625);

    CommonViews::ShaderCheckbox(settings->verticalFlip);
    CommonViews::ShaderCheckbox(settings->horizontalFlip);
    
    // Add mode selector
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 5.0);
    CommonViews::ShaderCheckbox(settings->center);
    CommonViews::Selector(settings->mode, settings->mode->options);
  }
  
  void clear() override {}
};

#endif
