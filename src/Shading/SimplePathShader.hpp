//
//  SimplePathShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef SimplePathShader_hpp
#define SimplePathShader_hpp

#include "ofMain.h"
#include "ShaderSettings.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "WaveformOscillator.hpp"
#include "Parameter.hpp"
#include "Shader.hpp"
#include <stdio.h>

struct SimplePathSettings: public ShaderSettings {
  std::shared_ptr<Parameter> shape;
  std::shared_ptr<Parameter> scale;
  std::shared_ptr<Parameter> xPosition;
  std::shared_ptr<Parameter> yPosition;
  std::shared_ptr<Parameter> fillColor;
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
  
  SimplePathSettings(std::string shaderId, json j) :
  shape(std::make_shared<Parameter>("Shape", 0, 0, 2)), // 0: Square, 1: Triangle, 2: Circle
  scale(std::make_shared<Parameter>("Scale", 1.0, 0.1, 5.0)),
  xPosition(std::make_shared<Parameter>("X Position", 0.5, 0.0, 1.0)),
  yPosition(std::make_shared<Parameter>("Y Position", 0.5, 0.0, 1.0)),
  fillColor(std::make_shared<Parameter>("FillColor", ParameterType_Color)),
  strokeColor(std::make_shared<Parameter>("StrokeColor", ParameterType_Color)),
  strokeWidth(std::make_shared<Parameter>("Stroke Width", 5.0, 0.0, 50.0)),
  rotate(std::make_shared<Parameter>("Rotate", 0.0, 0.0, 360.0)),
  autoRotate(std::make_shared<Parameter>("Auto-Rotate", 0.0, 0.0, 2.0)),
  xPositionOscillator(std::make_shared<WaveformOscillator>(xPosition)),
  yPositionOscillator(std::make_shared<WaveformOscillator>(yPosition)),
  scaleOscillator(std::make_shared<WaveformOscillator>(scale)),
  strokeWidthOscillator(std::make_shared<WaveformOscillator>(strokeWidth)),
  rotateOscillator(std::make_shared<WaveformOscillator>(rotate)),
  autoRotateOscillator(std::make_shared<WaveformOscillator>(autoRotate)),
  ShaderSettings(shaderId, j, "SimplePath") {
    parameters = { shape, scale, xPosition, yPosition, fillColor, strokeColor, strokeWidth, rotate, autoRotate };
    oscillators = { yPositionOscillator, xPositionOscillator, strokeWidthOscillator, rotateOscillator, autoRotateOscillator };
    strokeColor->setColor({1.0, 1.0, 0.0, 1.0});
    load(j);
    registerParameters();
  };
};

struct SimplePathShader: Shader {
  SimplePathSettings *settings;
  SimplePathShader(SimplePathSettings *settings) : settings(settings), Shader(settings) {};
  
  void setup() override {
    shader.load("shaders/SimplePath");
  }

  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override {
    canvas->begin();
    ofClear(0, 0, 0, 0); // Clear the canvas

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
    return ShaderTypeSimplePath;
  }

  void drawSettings() override {
    CommonViews::H3Title("SimplePath");

    CommonViews::Selector(settings->shape, {"Square", "Triangle", "Circle"});
    CommonViews::ShaderParameter(settings->scale, nullptr);
    CommonViews::MultiSlider("Position", settings->xPosition->paramId, settings->xPosition, settings->yPosition, settings->xPositionOscillator, settings->yPositionOscillator);
    CommonViews::ShaderColor(settings->fillColor);
    CommonViews::ShaderColor(settings->strokeColor);
    CommonViews::ShaderParameter(settings->strokeWidth, settings->strokeWidthOscillator);
    CommonViews::ShaderParameter(settings->rotate, settings->rotateOscillator);
    CommonViews::ShaderParameter(settings->autoRotate, settings->autoRotateOscillator);
  }
};

#endif /* SimplePathShader_hpp */
