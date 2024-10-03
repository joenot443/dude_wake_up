//
//  MultiMixShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef MultiMixShader_hpp
#define MultiMixShader_hpp

#include "ofMain.h"
#include "ShaderSettings.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "ValueOscillator.hpp"
#include "Parameter.hpp"
#include "Shader.hpp"
#include <stdio.h>
#include <algorithm>

struct MultiMixSettings: public ShaderSettings {
  std::shared_ptr<Parameter> mix0;
  std::shared_ptr<Parameter> mix1;
  std::shared_ptr<Parameter> mix2;
  std::shared_ptr<Parameter> mix3;
  std::shared_ptr<Parameter> mix4;
  std::shared_ptr<Parameter> mix5;
  std::shared_ptr<Parameter> mix6;
  std::shared_ptr<Parameter> mix7;
  std::shared_ptr<Parameter> mix8;
  std::shared_ptr<Parameter> mix9;
  
  std::map<InputSlot, std::shared_ptr<Parameter>> slotMixMap;
  
  std::vector<std::shared_ptr<Parameter>> mixs;
  
  MultiMixSettings(std::string shaderId, json j) :
  mix0(std::make_shared<Parameter>("Mix 1", 0.0, 0.0, 1.0)),
  mix1(std::make_shared<Parameter>("Mix 2", 0.0, 0.0, 1.0)),
  mix2(std::make_shared<Parameter>("Mix 3", 0.0, 0.0, 1.0)),
  mix3(std::make_shared<Parameter>("Mix 4", 0.0, 0.0, 1.0)),
  mix4(std::make_shared<Parameter>("Mix 5", 0.0, 0.0, 1.0)),
  mix5(std::make_shared<Parameter>("Mix 6", 0.0, 0.0, 1.0)),
  mix6(std::make_shared<Parameter>("Mix 7", 0.0, 0.0, 1.0)),
  mix7(std::make_shared<Parameter>("Mix 8", 0.0, 0.0, 1.0)),
  mix8(std::make_shared<Parameter>("Mix 9", 0.0, 0.0, 1.0)),
  mix9(std::make_shared<Parameter>("Mix 10", 0.0, 0.0, 1.0)),
  ShaderSettings(shaderId, j, "MultiMix") {
    slotMixMap = { {InputSlotMain, mix0, }, {InputSlotTwo, mix1}, {InputSlotThree, mix2}, {InputSlotFour, mix3}, {InputSlotFive, mix4}, {InputSlotSix, mix5}, {InputSlotSeven, mix6}, {InputSlotEight, mix7}, {InputSlotNine, mix8} };
    parameters = { mix0, mix1, mix2, mix3, mix4, mix5, mix6, mix7, mix8, mix9 };
    oscillators = {};
    load(j);
    registerParameters();
  };
};

struct MultiMixShader: Shader {
  MultiMixSettings *settings;
  MultiMixShader(MultiMixSettings *settings) : settings(settings), Shader(settings) {};

  // Bool array of active inputs
  std::set<InputSlot> activeInputs;
  int lastFrameInputCount = 0;
  
  void setup() override {
    shader.load("shaders/MultiMix");
  }
  
  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override {
    activeInputs.clear();
    ofEnableAlphaBlending();
    canvas->begin();
    shader.begin();
    // Clear the frame
    ofClear(0,0,0, 255);
    ofClear(0,0,0, 0);
    clearMixUniforms();
    
    // Accumulate the connected inputs
    for (auto [slot, conn] : inputs) {
      activeInputs.insert(slot);
      loadTextureAt(slot);
    }
    
    if (lastFrameInputCount != activeInputs.size()) { resetSliders(); }
    lastFrameInputCount = activeInputs.size();
    
    shader.setUniform1f("time", ofGetElapsedTimef());
    shader.setUniform2f("dimensions", frame->getWidth(), frame->getHeight());
    frame->draw(0, 0);
    shader.end();
    canvas->end();
  }
  
  void clearMixUniforms() {
    for (InputSlot slot : AllInputSlots) {
      auto mixName = "mix_" + ofToString(slot);
      shader.setUniform1f(mixName, 0.0f);
    }
  }
  
  void loadTextureAt(InputSlot slot) {
    auto name = "tex_" + ofToString(slot);
    auto mixName = "mix_" + ofToString(slot);
    ofTexture& tex = inputAtSlot(slot)->frame()->getTexture();
    
    int textureUnit = 8 + activeInputs.size() * 4;
    tex.bind(textureUnit);
    shader.setUniformTexture(name, tex, textureUnit);
    shader.setUniform1f(mixName, settings->slotMixMap[slot]->value);
    //    tex.unbind(textureUnit);
  }
  
  void settleSliders(InputSlot slot) {
      // Don't settle when there's only one input
      if (activeInputs.size() <= 1) return;

      // Calculate the total sum not including the changed slot
      float remainingSum = 0.0f;
      for (InputSlot i_slot : activeInputs) {
          if (slot != i_slot) {
              remainingSum += settings->slotMixMap[i_slot]->value;
          }
      }

      float changedValue = settings->slotMixMap[slot]->value;
      float newSum = 1.0f - changedValue;

      // If there's remaining sum, scale the other sliders
      if (remainingSum > 0.0) {
          float scale = newSum / remainingSum;
          for (InputSlot i_slot : activeInputs) {
              if (slot != i_slot) {
                  float newValue = settings->slotMixMap[i_slot]->value * scale;
                  settings->slotMixMap[i_slot]->value = newValue;
              }
          }
      } else {
          // If the old sum is zero, redistribute equally
          float evenValue = newSum / (activeInputs.size() - 1);
          for (InputSlot i_slot : activeInputs) {
              if (slot != i_slot) {
                  settings->slotMixMap[i_slot]->value = evenValue;
              }
          }
      }
  }

  
  void resetSliders() {
    float splitValue = 1.0f / activeInputs.size();
    for (auto [slot, param] : settings->slotMixMap) {
      // If the input is in activeInputs, set it to splitValue.
      // Otherwise, set to 0.
      param->value = activeInputs.count(slot) ? splitValue : 0.0f;
    }
  }
  
  void clear() override {
    
  }
  
  int inputCount() override {
    return std::max(2, std::min(10, static_cast<int>(activeInputs.size() + 1)));
  }
  
  ShaderType type() override {
    return ShaderTypeMultiMix;
  }
  
  void drawSettings() override {
    CommonViews::H3Title("MultiMix");
    
    for (InputSlot slot : activeInputs) {
      if (CommonViews::ShaderParameter(settings->slotMixMap[slot], nullptr)) {
        settleSliders(slot);
      }
    }
  }
};

#endif /* MultiMixShader_hpp */
