//
//  SimpleBarsShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef SimpleBarsShader_hpp
#define SimpleBarsShader_hpp

#include "ofMain.h"
#include "ShaderSettings.hpp"
#include "CommonViews.hpp"
#include "LayoutStateService.hpp"
#include "ofxImGui.h"
#include "WaveformOscillator.hpp"
#include "Parameter.hpp"
#include "Shader.hpp"
#include <stdio.h>

struct SimpleBarsSettings: public ShaderSettings {
  std::shared_ptr<Parameter> orientation; // 0: Horizontal, 1: Vertical
  std::shared_ptr<Parameter> barCount;
  std::shared_ptr<Parameter> barWidth;
  std::shared_ptr<Parameter> barSpacing;
  std::shared_ptr<Parameter> fillColor;
  
  std::shared_ptr<WaveformOscillator> barCountOscillator;
  std::shared_ptr<WaveformOscillator> barWidthOscillator;
  std::shared_ptr<WaveformOscillator> barSpacingOscillator;
  
  SimpleBarsSettings(std::string shaderId, json j) :
  orientation(std::make_shared<Parameter>("Orientation", 0, 0, 1)), // 0: Horizontal, 1: Vertical
  barCount(std::make_shared<Parameter>("Bar Count", 10, 1, 100)),
  barWidth(std::make_shared<Parameter>("Bar Width", 100.0, 1.0, 500.0)),
  barSpacing(std::make_shared<Parameter>("Bar Spacing", 20.0, 0.0, 200.0)),
  fillColor(std::make_shared<Parameter>("FillColor", ParameterType_Color)),
  barCountOscillator(std::make_shared<WaveformOscillator>(barCount)),
  barWidthOscillator(std::make_shared<WaveformOscillator>(barWidth)),
  barSpacingOscillator(std::make_shared<WaveformOscillator>(barSpacing)),
  ShaderSettings(shaderId, j, "SimpleBars") {
    parameters = { orientation, barCount, barWidth, barSpacing, fillColor };
    oscillators = { barCountOscillator, barWidthOscillator, barSpacingOscillator };
    fillColor->setColor({0.0, 1.0, 0.0, 1.0});
    load(j);
    registerParameters();
  };
};

struct SimpleBarsShader: Shader {
  SimpleBarsSettings *settings;
  SimpleBarsShader(SimpleBarsSettings *settings) : settings(settings), Shader(settings) {};
  
  void setup() override {
    shader.load("shaders/SimpleBars");
  }
  
  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override {
    canvas->begin();
    ofClear(0, 0, 0, 0); // Clear the canvas
    
    int count = static_cast<int>(settings->barCount->value);
    float width = settings->barWidth->value;
    float spacing = settings->barSpacing->value;
    bool isHorizontal = settings->orientation->intValue == 0;
    
    ofSetColor(settings->fillColor->color->data()[0] * 255, settings->fillColor->color->data()[1] * 255, settings->fillColor->color->data()[2] * 255, settings->fillColor->color->data()[3] * 255);
    
    float centerPosition = isHorizontal ? frame->getHeight() / 2 : frame->getWidth() / 2;
    
    for (int i = 0; i < count; ++i) {
      float offset = (i - count / 2) * (width + spacing);
      float position = centerPosition + offset;
      
      if (isHorizontal) {
        ofDrawRectangle(0, position, frame->getWidth(), width);
      } else {
        ofDrawRectangle(position, 0, width, frame->getHeight());
      }
    }
    
    canvas->end();
  }
  
  void clear() override {
    
  }
  
  int inputCount() override {
    return 1;
  }
  
  ShaderType type() override {
    return ShaderTypeSimpleBars;
  }
  
  void drawSettings() override {
    
    if (ImGui::Button("Randomize Bars")) {
      random();
    }
    ImGui::SameLine();
    if (ImGui::Button("Randomize Oscillators")) {
      randomizeOscillators();
    }
    
    CommonViews::Selector(settings->orientation, {"Horizontal", "Vertical"});
    CommonViews::ShaderParameter(settings->barCount, settings->barCountOscillator);
    CommonViews::ShaderParameter(settings->barWidth, settings->barWidthOscillator);
    CommonViews::ShaderParameter(settings->barSpacing, settings->barSpacingOscillator);
    CommonViews::ShaderColor(settings->fillColor);
  }
  
  void random() {
    settings->orientation->setValue(ofRandom(0, 2)); // Random orientation: 0 or 1
    settings->barCount->setValue(ofRandom(1, 100)); // Random bar count between 1 and 100
    settings->barWidth->setValue(ofRandom(1.0, 50.0)); // Random bar width between 1.0 and 50.0
    settings->barSpacing->setValue(ofRandom(0.0, 20.0)); // Random bar spacing between 0.0 and 20.0
    settings->fillColor->setColor({ofRandom(0.0, 1.0), ofRandom(0.0, 1.0), ofRandom(0.0, 1.0), 1.0}); // Random fill color
  }
  
  void randomizeOscillators() {
    LayoutStateService::getService()->utilityPanelTab = 1;
    for (auto& oscillator : settings->oscillators) {
      if (ofRandom(1.0) < 0.5) { // 50% chance to enable
        if (oscillator->type() == OscillatorType_waveform) {
          auto waveform = std::static_pointer_cast<WaveformOscillator>(oscillator);
          waveform->enabled->setBoolValue(true);
          waveform->frequency->setValue(ofRandom(waveform->frequency->min / 2.0, waveform->frequency->max / 2.0));
          waveform->shift->setValue(ofRandom(waveform->shift->min / 3.0, waveform->shift->max / 3.0));
          waveform->amplitude->setValue(ofRandom(waveform->amplitude->min / 2.0, waveform->amplitude->max / 2.0));
        } else {
          oscillator->enabled->setBoolValue(false);
        }
      }
    }
  }
};

#endif /* SimpleBarsShader_hpp */
