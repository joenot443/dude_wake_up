//
//  ScrapbookShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 2024.
//

#ifndef ScrapbookShader_hpp
#define ScrapbookShader_hpp

#include "ofMain.h"
#include "ShaderSettings.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "ValueOscillator.hpp"
#include "Parameter.hpp"
#include "Shader.hpp"
#include <stdio.h>

struct ScrapbookSettings : public ShaderSettings {
  std::shared_ptr<Parameter> interval;
  std::shared_ptr<Parameter> minSize;
  std::shared_ptr<Parameter> maxSize;
  std::shared_ptr<Parameter> accumulate;

  std::shared_ptr<WaveformOscillator> intervalOscillator;
  std::shared_ptr<WaveformOscillator> minSizeOscillator;
  std::shared_ptr<WaveformOscillator> maxSizeOscillator;

  ScrapbookSettings(std::string shaderId, json j)
      : interval(std::make_shared<Parameter>("Interval", 0.5, 0.05, 5.0)),
        minSize(std::make_shared<Parameter>("Min Size", 0.1, 0.01, 1.0)),
        maxSize(std::make_shared<Parameter>("Max Size", 0.5, 0.01, 1.0)),
        accumulate(std::make_shared<Parameter>("Accumulate", 0.0, 0.0, 1.0, ParameterType_Bool)),
        intervalOscillator(std::make_shared<WaveformOscillator>(interval)),
        minSizeOscillator(std::make_shared<WaveformOscillator>(minSize)),
        maxSizeOscillator(std::make_shared<WaveformOscillator>(maxSize)),
        ShaderSettings(shaderId, j, "Scrapbook") {
    parameters = {interval, minSize, maxSize, accumulate};
    oscillators = {intervalOscillator, minSizeOscillator, maxSizeOscillator};
    load(j);
    registerParameters();
  };
};

struct ScrapbookShader : Shader {
  ScrapbookSettings *settings;
  ofFbo accumulationFbo;
  float lastDrawTime = 0.0f;
  bool needsInitialClear = true;

  ScrapbookShader(ScrapbookSettings *settings)
      : settings(settings), Shader(settings) {};

  void setup() override {
    // No GLSL shader to load - pure OpenFrameworks rendering
  }

  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override {
    if (!frame || !canvas || !frame->isAllocated() || !canvas->isAllocated())
      return;

    float w = canvas->getWidth();
    float h = canvas->getHeight();

    // Allocate accumulation FBO if needed
    if (!accumulationFbo.isAllocated() ||
        accumulationFbo.getWidth() != w ||
        accumulationFbo.getHeight() != h) {
      accumulationFbo.allocate(w, h, GL_RGBA);
      accumulationFbo.begin();
      ofClear(0, 0, 0, 255);
      accumulationFbo.end();
      needsInitialClear = true;
    }

    if (needsInitialClear) {
      accumulationFbo.begin();
      ofClear(0, 0, 0, 255);
      accumulationFbo.end();
      needsInitialClear = false;
      lastDrawTime = ofGetElapsedTimef();
    }

    float currentTime = ofGetElapsedTimef();
    float intervalVal = settings->interval->value;

    if (currentTime - lastDrawTime >= intervalVal) {
      lastDrawTime = currentTime;

      if (!settings->accumulate->boolValue) {
        accumulationFbo.begin();
        ofClear(0, 0, 0, 255);
        accumulationFbo.end();
      }

      float minSizeVal = settings->minSize->value;
      float maxSizeVal = settings->maxSize->value;
      if (minSizeVal > maxSizeVal) std::swap(minSizeVal, maxSizeVal);

      accumulationFbo.begin();

      // Draw the main frame input
      if (frame->getTexture().getTextureData().textureID != 0) {
        float sizeFactor = ofRandom(minSizeVal, maxSizeVal);
        float drawW = w * sizeFactor;
        float drawH = h * sizeFactor;
        float drawX = ofRandom(-drawW * 0.5f, w - drawW * 0.5f);
        float drawY = ofRandom(-drawH * 0.5f, h - drawH * 0.5f);
        frame->draw(drawX, drawY, drawW, drawH);
      }

      // Draw each additional connected input at a random position and size
      for (InputSlot slot : AllInputSlots) {
        if (slot == InputSlotMain) continue;
        if (!hasInputAtSlot(slot)) continue;

        std::shared_ptr<ofFbo> inputFrame = inputAtSlot(slot)->frame();
        if (!inputFrame || !inputFrame->isAllocated()) continue;
        if (inputFrame->getTexture().getTextureData().textureID == 0) continue;

        float sizeFactor = ofRandom(minSizeVal, maxSizeVal);
        float drawW = w * sizeFactor;
        float drawH = h * sizeFactor;

        float drawX = ofRandom(-drawW * 0.5f, w - drawW * 0.5f);
        float drawY = ofRandom(-drawH * 0.5f, h - drawH * 0.5f);

        inputFrame->draw(drawX, drawY, drawW, drawH);
      }

      accumulationFbo.end();
    }

    // Output the accumulation buffer
    canvas->begin();
    accumulationFbo.draw(0, 0);
    canvas->end();
  }

  void clear() override {
    if (accumulationFbo.isAllocated()) {
      accumulationFbo.begin();
      ofClear(0, 0, 0, 255);
      accumulationFbo.end();
    }
    needsInitialClear = true;
  }

  int inputCount() override {
    return inputs.size() + 1;
  }

  ShaderType type() override {
    return ShaderTypeScrapbook;
  }

  void drawSettings() override {
    CommonViews::H3Title("Scrapbook");
    CommonViews::ShaderParameter(settings->interval, settings->intervalOscillator);
    CommonViews::ShaderParameter(settings->minSize, settings->minSizeOscillator);
    CommonViews::ShaderParameter(settings->maxSize, settings->maxSizeOscillator);
    CommonViews::ShaderCheckbox(settings->accumulate);
  }
};

#endif
