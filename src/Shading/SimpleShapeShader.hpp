//
//  SimpleShapeShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef SimpleShapeShader_hpp
#define SimpleShapeShader_hpp

#include "LayoutStateService.hpp"
#include "ofMain.h"
#include "ShaderSettings.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "WaveformOscillator.hpp"
#include "Parameter.hpp"
#include "Shader.hpp"
#include <stdio.h>

struct SimpleShapeSettings: public ShaderSettings {
  std::shared_ptr<Parameter> shape;
  std::shared_ptr<Parameter> scale;
  std::shared_ptr<Parameter> xPosition;
  std::shared_ptr<Parameter> yPosition;
  std::shared_ptr<Parameter> fillColor;
  std::shared_ptr<Parameter> backgroundColor;
  std::shared_ptr<Parameter> strokeColor;
  std::shared_ptr<Parameter> strokeWidth;
  std::shared_ptr<Parameter> rotate;
  std::shared_ptr<Parameter> autoRotate;
  
  std::shared_ptr<WaveformOscillator> xPositionOscillator;
  std::shared_ptr<WaveformOscillator> yPositionOscillator;
  std::shared_ptr<WaveformOscillator> scaleOscillator;
  std::shared_ptr<WaveformOscillator> strokeWidthOscillator;
  std::shared_ptr<WaveformOscillator> rotateOscillator;
  std::shared_ptr<WaveformOscillator> autoRotateOscillator;
  
  SimpleShapeSettings(std::string shaderId, json j) :
  shape(std::make_shared<Parameter>("Shape", 0, 0, 2)), // 0: Square, 1: Triangle, 2: Circle
  scale(std::make_shared<Parameter>("Scale", 3.0, 0.1, 100.0)),
  xPosition(std::make_shared<Parameter>("X Position", 0.5, 0.0, 1.0)),
  yPosition(std::make_shared<Parameter>("Y Position", 0.5, 0.0, 1.0)),
  fillColor(std::make_shared<Parameter>("FillColor", ParameterType_Color)),
  backgroundColor(std::make_shared<Parameter>("Background Color", ParameterType_Color)),
  strokeColor(std::make_shared<Parameter>("StrokeColor", ParameterType_Color)),
  strokeWidth(std::make_shared<Parameter>("Stroke Width", 10.0, 0.0, 50.0)),
  rotate(std::make_shared<Parameter>("Rotate", 0.0, 0.0, 360.0)),
  autoRotate(std::make_shared<Parameter>("Auto-Rotate", 0.0, 0.0, 2.0)),
  xPositionOscillator(std::make_shared<WaveformOscillator>(xPosition)),
  yPositionOscillator(std::make_shared<WaveformOscillator>(yPosition)),
  scaleOscillator(std::make_shared<WaveformOscillator>(scale)),
  strokeWidthOscillator(std::make_shared<WaveformOscillator>(strokeWidth)),
  rotateOscillator(std::make_shared<WaveformOscillator>(rotate)),
  autoRotateOscillator(std::make_shared<WaveformOscillator>(autoRotate)),
  ShaderSettings(shaderId, j, "SimpleShape") {
    parameters = { shape, scale, xPosition, yPosition, fillColor, backgroundColor, strokeColor, strokeWidth, rotate, autoRotate };
    oscillators = { yPositionOscillator, xPositionOscillator, strokeWidthOscillator, rotateOscillator, autoRotateOscillator };
    strokeColor->setColor({1.0, 0.5, 0.0, 1.0});
    backgroundColor->setColor({0.0, 0.0, 0.0, 1.0});
    load(j);
    registerParameters();
  };
};

struct SimpleShapeShader: Shader {
  SimpleShapeSettings *settings;
  SimpleShapeShader(SimpleShapeSettings *settings) : settings(settings), Shader(settings) {};
  
  void setup() override {
  }
  
  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override {
    canvas->begin();
    ofClear(0, 0, 0, 0);
    ofClear(settings->backgroundColor->color->data()[0] * 255, settings->backgroundColor->color->data()[1] * 255, settings->backgroundColor->color->data()[2] * 255, settings->backgroundColor->color->data()[3] * 255); // Clear the canvas
    
    float x = settings->xPosition->value * frame->getWidth();
    float y = settings->yPosition->value * frame->getHeight();
    float size = settings->scale->value * 50; // Arbitrary scale factor
    float strokeSize = size + settings->strokeWidth->value; // Use strokeWidth parameter
    float rotation = settings->rotate->value + ofGetElapsedTimef() * settings->autoRotate->value * 50; // Use rotate and autoRotate parameters
    
    ofPushMatrix();
    ofTranslate(x, y);
    ofRotateDeg(rotation);
    
    
    // Draw stroke
    ofSetColor(settings->strokeColor->color->data()[0] * 255, settings->strokeColor->color->data()[1] * 255, settings->strokeColor->color->data()[2] * 255, settings->strokeColor->color->data()[3] * 255);
    switch (static_cast<int>(settings->shape->intValue)) {
      case 0: // Square
        ofDrawRectangle(-strokeSize / 2, -strokeSize / 2, strokeSize, strokeSize);
        break;
      case 1: // Triangle
        ofDrawTriangle(0, -strokeSize / 2, -strokeSize / 2, strokeSize / 2, strokeSize / 2, strokeSize / 2);
        break;
      case 2: // Circle
        ofSetCircleResolution(100);
        ofDrawCircle(0, 0, strokeSize / 2);
        break;
    }
    
    // Draw fill
    ofSetColor(settings->fillColor->color->data()[0] * 255, settings->fillColor->color->data()[1] * 255, settings->fillColor->color->data()[2] * 255, settings->fillColor->color->data()[3] * 255);
    switch (static_cast<int>(settings->shape->intValue)) {
      case 0: // Square
        ofDrawRectangle(-size / 2, -size / 2, size, size);
        break;
      case 1: // Triangle
        ofDrawTriangle(0, -size / 2, -size / 2, size / 2, size / 2, size / 2);
        break;
      case 2: // Circle
        ofDrawCircle(0, 0, size / 2);
        break;
    }
    
    ofPopMatrix();
    canvas->end();
  }
  
  void clear() override {
    
  }
  
  int inputCount() override {
    return 1;
  }
  
  ShaderType type() override {
    return ShaderTypeSimpleShape;
  }
  
  void drawSettings() override {
    
    if (ImGui::Button("Randomize Shape")) {
      random();
    }
    ImGui::SameLine();
    if (ImGui::Button("Randomize Oscillators")) {
      randomizeOscillators();
    }
    
    CommonViews::Selector(settings->shape, {"Square", "Triangle", "Circle"});
    CommonViews::ShaderParameter(settings->scale, nullptr);
    CommonViews::MultiSlider("Position", settings->xPosition->paramId, settings->xPosition, settings->yPosition, settings->xPositionOscillator, settings->yPositionOscillator, 0.5625);
    CommonViews::ShaderColor(settings->fillColor);
    CommonViews::ShaderColor(settings->strokeColor);
    CommonViews::ShaderColor(settings->backgroundColor);
    CommonViews::ShaderParameter(settings->strokeWidth, settings->strokeWidthOscillator);
    CommonViews::ShaderParameter(settings->rotate, settings->rotateOscillator);
    CommonViews::ShaderParameter(settings->autoRotate, settings->autoRotateOscillator);
    
  }
  
  void random() {
    settings->shape->setValue(ofRandom(0, 3)); // Random shape: 0, 1, or 2
    settings->scale->setValue(ofRandom(0.1, 5.0)); // Random scale between 0.1 and 5.0
    settings->xPosition->setValue(ofRandom(0.0, 1.0)); // Random x position between 0.0 and 1.0
    settings->yPosition->setValue(ofRandom(0.0, 1.0)); // Random y position between 0.0 and 1.0
    settings->fillColor->setColor({ofRandom(0.0, 1.0), ofRandom(0.0, 1.0), ofRandom(0.0, 1.0), 1.0}); // Random fill color
    settings->strokeColor->setColor({ofRandom(0.0, 1.0), ofRandom(0.0, 1.0), ofRandom(0.0, 1.0), 1.0}); // Random stroke color
    settings->strokeWidth->setValue(ofRandom(0.0, 50.0)); // Random stroke width between 0.0 and 50.0
    settings->rotate->setValue(ofRandom(0.0, 360.0)); // Random rotation between 0.0 and 360.0
    settings->autoRotate->setValue(ofRandom(0.0, 2.0)); // Random auto-rotate between 0.0 and 2.0
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

#endif
