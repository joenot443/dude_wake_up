//
//  BubblesShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef BubblesShader_hpp
#define BubblesShader_hpp

#include "ofMain.h"
#include "ShaderSettings.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "TimeService.hpp"
#include "WaveformOscillator.hpp"
#include "Parameter.hpp"
#include "Shader.hpp"
#include <stdio.h>

struct BubblesSettings: public ShaderSettings {
  // Core visual parameters
  std::shared_ptr<Parameter> maxIterations;    // Number of bubble layers
  std::shared_ptr<Parameter> nearPlane;        // Near plane distance
  std::shared_ptr<Parameter> fadeIntensity;    // How quickly bubbles fade with distance
  std::shared_ptr<Parameter> bubbleOpacity;    // Overall bubble transparency

  // Effect parameters
  std::shared_ptr<Parameter> bubbleSize;       // Base size of bubbles
  std::shared_ptr<Parameter> bubbleSpeed;      // Animation speed
  std::shared_ptr<Parameter> colorIntensity;   // Color saturation
  std::shared_ptr<Parameter> lightIntensity;   // Lighting effect strength

  // Core oscillators
  std::shared_ptr<WaveformOscillator> maxIterationsOscillator;
  std::shared_ptr<WaveformOscillator> nearPlaneOscillator;
  std::shared_ptr<WaveformOscillator> fadeIntensityOscillator;
  std::shared_ptr<WaveformOscillator> bubbleOpacityOscillator;

  // Effect oscillators
  std::shared_ptr<WaveformOscillator> bubbleSizeOscillator;
  std::shared_ptr<WaveformOscillator> bubbleSpeedOscillator;
  std::shared_ptr<WaveformOscillator> colorIntensityOscillator;
  std::shared_ptr<WaveformOscillator> lightIntensityOscillator;

  BubblesSettings(std::string shaderId, json j) :
  // Initialize core parameters
  maxIterations(std::make_shared<Parameter>("Max Iterations", 12.0, 4.0, 24.0)),
  nearPlane(std::make_shared<Parameter>("Near Plane", 4.0, 1.0, 8.0)),
  fadeIntensity(std::make_shared<Parameter>("Fade Intensity", 0.25, 0.1, 1.0)),
  bubbleOpacity(std::make_shared<Parameter>("Bubble Opacity", 1.0, 0.1, 1.0)),
  
  // Initialize effect parameters
  bubbleSize(std::make_shared<Parameter>("Bubble Size", 0.475, 0.1, 1.0)),
  bubbleSpeed(std::make_shared<Parameter>("Bubble Speed", 0.25, 0.1, 1.0)),
  colorIntensity(std::make_shared<Parameter>("Color Intensity", 1.5, 0.5, 3.0)),
  lightIntensity(std::make_shared<Parameter>("Light Intensity", 2.0, 0.5, 4.0)),
  
  // Initialize core oscillators
  maxIterationsOscillator(std::make_shared<WaveformOscillator>(maxIterations)),
  nearPlaneOscillator(std::make_shared<WaveformOscillator>(nearPlane)),
  fadeIntensityOscillator(std::make_shared<WaveformOscillator>(fadeIntensity)),
  bubbleOpacityOscillator(std::make_shared<WaveformOscillator>(bubbleOpacity)),
  
  // Initialize effect oscillators
  bubbleSizeOscillator(std::make_shared<WaveformOscillator>(bubbleSize)),
  bubbleSpeedOscillator(std::make_shared<WaveformOscillator>(bubbleSpeed)),
  colorIntensityOscillator(std::make_shared<WaveformOscillator>(colorIntensity)),
  lightIntensityOscillator(std::make_shared<WaveformOscillator>(lightIntensity)),
  
  ShaderSettings(shaderId, j, "Bubbles") {
    parameters = { 
      maxIterations, nearPlane, fadeIntensity, bubbleOpacity,
      bubbleSize, bubbleSpeed, colorIntensity, lightIntensity 
    };
    oscillators = { 
      maxIterationsOscillator, nearPlaneOscillator, fadeIntensityOscillator, bubbleOpacityOscillator,
      bubbleSizeOscillator, bubbleSpeedOscillator, colorIntensityOscillator, lightIntensityOscillator 
    };
    load(j);
    registerParameters();
  };
};

struct BubblesShader: Shader {
  BubblesSettings *settings;
  BubblesShader(BubblesSettings *settings) : settings(settings), Shader(settings) {};
  
  void setup() override {
    shader.load("shaders/Bubbles");
  }

  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override {
    canvas->begin();
    shader.begin();
    shader.setUniformTexture("tex", frame->getTexture(), 4);
    shader.setUniform1f("time", TimeService::getService()->timeParam->value);
    shader.setUniform2f("dimensions", frame->getWidth(), frame->getHeight());
    
    // Pass core parameters to shader
    shader.setUniform1f("maxIterations", settings->maxIterations->value);
    shader.setUniform1f("nearPlane", settings->nearPlane->value);
    shader.setUniform1f("fadeIntensity", settings->fadeIntensity->value);
    shader.setUniform1f("bubbleOpacity", settings->bubbleOpacity->value);
    
    // Pass effect parameters to shader
    shader.setUniform1f("bubbleSize", settings->bubbleSize->value);
    shader.setUniform1f("bubbleSpeed", settings->bubbleSpeed->value);
    shader.setUniform1f("colorIntensity", settings->colorIntensity->value);
    shader.setUniform1f("lightIntensity", settings->lightIntensity->value);
    
    frame->draw(0, 0);
    shader.end();
    canvas->end();
  }

  void clear() override {
  }

  int inputCount() override {
    return 1;
  }

  ShaderType type() override {
    return ShaderTypeBubbles;
  }

  void drawSettings() override {
    CommonViews::H3Title("Bubbles");
    
    // Core visual parameters
    CommonViews::H4Title("Core Parameters");
    CommonViews::ShaderParameter(settings->maxIterations, settings->maxIterationsOscillator);
    CommonViews::ShaderParameter(settings->nearPlane, settings->nearPlaneOscillator);
    CommonViews::ShaderParameter(settings->fadeIntensity, settings->fadeIntensityOscillator);
    CommonViews::ShaderParameter(settings->bubbleOpacity, settings->bubbleOpacityOscillator);
    
    // Effect parameters
    CommonViews::H4Title("Effect Parameters");
    CommonViews::ShaderParameter(settings->bubbleSize, settings->bubbleSizeOscillator);
    CommonViews::ShaderParameter(settings->bubbleSpeed, settings->bubbleSpeedOscillator);
    CommonViews::ShaderParameter(settings->colorIntensity, settings->colorIntensityOscillator);
    CommonViews::ShaderParameter(settings->lightIntensity, settings->lightIntensityOscillator);
  }
};

#endif /* BubblesShader_hpp */