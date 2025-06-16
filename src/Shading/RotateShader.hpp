//
//  RotateShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef RotateShader_hpp
#define RotateShader_hpp

#include "ofMain.h"
#include "ShaderSettings.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "ValueOscillator.hpp"
#include "Parameter.hpp"
#include "Shader.hpp"
#include <stdio.h>

struct RotateSettings: public ShaderSettings {
  std::shared_ptr<Parameter> rotate;
  std::shared_ptr<Parameter> autoRotate;
  std::shared_ptr<Parameter> center;
  std::shared_ptr<Parameter> scale;
  std::shared_ptr<Parameter> verticalFlip;
  std::shared_ptr<Parameter> horizontalFlip;
  
  std::shared_ptr<Oscillator> scaleOscillator;
  std::shared_ptr<Oscillator> rotateOscillator;
  std::shared_ptr<Oscillator> autoRotateOscillator;
  
  RotateSettings(std::string shaderId, json j) :
  rotate(std::make_shared<Parameter>("Rotate", 0.0, 0.0, 360.0)),
  verticalFlip(std::make_shared<Parameter>("Vertical Flip", 0.0, 0.0, 1.0, ParameterType_Bool)),
  horizontalFlip(std::make_shared<Parameter>("Horizontal Flip", 0.0, 0.0, 1.0, ParameterType_Bool)),
  center(std::make_shared<Parameter>("Center", 0.0, 0.0, 1.0, ParameterType_Bool)),
  scale(std::make_shared<Parameter>("Scale", 1.0, 0.0, 5.0)),
  autoRotate(std::make_shared<Parameter>("Auto-Rotate", 0.0, 0.0, 2.0, ParameterType_Bool)),
  rotateOscillator(std::make_shared<WaveformOscillator>(rotate)),
  scaleOscillator(std::make_shared<WaveformOscillator>(scale)),
  autoRotateOscillator(std::make_shared<WaveformOscillator>(autoRotate)),
  ShaderSettings(shaderId, j, "Rotate") {
    parameters = {rotate, scale, autoRotate, center};
    oscillators = {rotateOscillator, scaleOscillator, autoRotateOscillator};
    audioReactiveParameter = rotate;
    load(j);
    registerParameters();
  };
};

struct RotateShader: Shader {
  RotateSettings *settings;
  RotateShader(RotateSettings *settings) : settings(settings), Shader(settings) {};

  void setup() override {
    shader.load("shaders/Rotate");
  }

  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override {
    canvas->begin();
    
    ofClear(0, 0, 0, 255);
    ofClear(0, 0, 0, 0);
    
    // Push the current transformation matrix onto the matrix stack
    ofPushMatrix();
    
    // Calculate rotation value based on settings
    float rotate = settings->rotate->value;
    if (settings->autoRotate->value > 0) {
        rotate += ofGetElapsedTimef() * settings->autoRotate->value * 50;
    }

    // Apply scale
    ofScale(settings->scale->value, settings->scale->value);

    // Apply rotation with the correct pivot point
    if (settings->center->value) {
      // Move origin to where you want the center of the frame to be (e.g. (0,0) or wherever you want the frame's center)
      ofTranslate(frame->getWidth() / 2, frame->getHeight() / 2);
      ofRotateDeg(rotate);
      // Draw the frame centered at the origin
      frame->draw(-frame->getWidth() / 2, -frame->getHeight() / 2);
    } else {
      // Rotate about top-left
      ofRotateDeg(rotate);
      frame->draw(0, 0);
    }

    if (settings->verticalFlip->value) {
      ofScale(1, -1);
    }

    if (settings->horizontalFlip->value) {
      ofScale(-1, 1);
    }

    // Restore the original transformation matrix
    ofPopMatrix();
    canvas->end();
  }

  void clear() override {
    
  }

  int inputCount() override {
    return 1;
  }

  ShaderType type() override {
    return ShaderTypeRotate;
  }

  void drawSettings() override {
    CommonViews::ShaderParameter(settings->scale, settings->scaleOscillator);
    CommonViews::ShaderParameter(settings->rotate, settings->rotateOscillator);
    CommonViews::ShaderParameter(settings->autoRotate, settings->autoRotateOscillator);
    CommonViews::ShaderCheckbox(settings->center);
    CommonViews::ShaderCheckbox(settings->verticalFlip);
    CommonViews::ShaderCheckbox(settings->horizontalFlip);
  }
};

#endif
