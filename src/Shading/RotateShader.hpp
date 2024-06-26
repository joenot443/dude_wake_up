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
  std::shared_ptr<Parameter> scale;
  std::shared_ptr<Oscillator> scaleOscillator;
  std::shared_ptr<Oscillator> rotateOscillator;
  std::shared_ptr<Oscillator> autoRotateOscillator;
  
  RotateSettings(std::string shaderId, json j) :
  rotate(std::make_shared<Parameter>("Rotate", 0.0, 0.0, 360.0)),
  scale(std::make_shared<Parameter>("Scale", 1.0, 0.0, 5.0)),
  autoRotate(std::make_shared<Parameter>("Auto-Rotate", 0.0, 0.0, 2.0)),
  rotateOscillator(std::make_shared<WaveformOscillator>(rotate)),
  scaleOscillator(std::make_shared<WaveformOscillator>(scale)),
  autoRotateOscillator(std::make_shared<WaveformOscillator>(autoRotate)),
  ShaderSettings(shaderId, j, "Rotate") {
    parameters = {rotate, scale, autoRotate};
    oscillators = {rotateOscillator, scaleOscillator, autoRotateOscillator};
    load(j);
    registerParameters();
  };
};

struct RotateShader: Shader {
  RotateSettings *settings;
  RotateShader(RotateSettings *settings) : settings(settings), Shader(settings) {};
  ofShader shader;
  void setup() override {
    shader.load("shaders/Rotate");
  }

  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override {
    canvas->begin();
    
    // Push the current transformation matrix onto the matrix stack
    ofPushMatrix();
    
    // Calculate rotation value based on settings
    float rotate = settings->rotate->value;
    if (settings->autoRotate->value > 0) {
        rotate += ofGetElapsedTimef() * settings->autoRotate->value * 50;
    }
    // Set the origin of transformations to the center of the frame
    ofTranslate(frame->getWidth() / 2, frame->getHeight() / 2);
    
    // Apply rotation
    ofRotateDeg(rotate);
    
    // Set the scale
    ofScale(settings->scale->value, settings->scale->value);
    
    // Move the origin back to the top-left corner before drawing
    ofTranslate(-frame->getWidth() / 2, -frame->getHeight() / 2);

    // Draw the frame at origin (which has been adjusted to the center)
    frame->draw(0, 0);
    
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
    CommonViews::H3Title("Rotate");

    CommonViews::ShaderParameter(settings->scale, settings->rotateOscillator);
    CommonViews::ShaderParameter(settings->rotate, settings->rotateOscillator);
    CommonViews::ShaderParameter(settings->autoRotate, settings->autoRotateOscillator);
  }
};

#endif /* RotateShader_hpp */
