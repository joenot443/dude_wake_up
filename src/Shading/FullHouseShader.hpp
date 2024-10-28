//
//  FullHouseShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef FullHouseShader_hpp
#define FullHouseShader_hpp

#include "ofMain.h"
#include "ShaderSettings.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "WaveformOscillator.hpp"
#include "Parameter.hpp"
#include "Shader.hpp"
#include <stdio.h>

struct FullHouseSettings: public ShaderSettings {
  std::shared_ptr<Parameter> count;
  std::shared_ptr<Parameter> length;
  std::shared_ptr<Parameter> speed;
  std::shared_ptr<WaveformOscillator> countOscillator;
  std::shared_ptr<WaveformOscillator> lengthOscillator;
  std::shared_ptr<WaveformOscillator> speedOscillator;
  
  FullHouseSettings(std::string shaderId, json j) :
  count(std::make_shared<Parameter>("Count", 10.0, 0.0, 100.0)),
  length(std::make_shared<Parameter>("Length", 120.0, 10.0, 300.0)),
  speed(std::make_shared<Parameter>("Speed", 50.0, 0.0, 500.0)),
  lengthOscillator(std::make_shared<WaveformOscillator>(length)),
  countOscillator(std::make_shared<WaveformOscillator>(count)),
  speedOscillator(std::make_shared<WaveformOscillator>(speed)),
  ShaderSettings(shaderId, j, "FullHouse") {
    parameters = { count, length, speed };
    oscillators = { countOscillator, lengthOscillator, speedOscillator };
    load(j);
    registerParameters();
  };
};

struct FullHouseShader: Shader {
  FullHouseSettings *settings;
  FullHouseShader(FullHouseSettings *settings) : settings(settings), Shader(settings) {};

  std::vector<glm::vec2> noiseSeeds;
  void setup() override {
    shader.load("shaders/FullHouse");
    // Initialize noise seeds only once
    for (int i = 0; i < 100; ++i) {
      noiseSeeds.push_back(glm::vec2(ofRandom(1000), ofRandom(1000)));
    }
  }
  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override {
    canvas->begin();
    
    ofTranslate(frame->getWidth() * 0.5, frame->getHeight() * 0.5);
    
    int numberOfActor = settings->count->value;
    auto len = settings->length->value;
    ofColor color;
    
    // Use a slower frame progression to manage movement
    float frameProgress = ofGetFrameNum() * 0.00001 * settings->speed->value;
    
    for (int i = 0; i < numberOfActor; i++) {
      glm::vec2 noise_seed = noiseSeeds[i];  // Use pre-initialized seeds
      
      // Draw the circles for each actor, creating a trail
      for (int k = 0; k < len; k++) {
        // Scale down the noise increments to make the trail smoother and more connected
        float noiseStep = k * 0.01;  // Adjust the scale factor to control the tightness of the trail
        
        auto location = glm::vec2(
                                  ofMap(ofNoise(noise_seed.x, noiseStep + frameProgress), 0, 1, -450, 450),
                                  ofMap(ofNoise(noise_seed.y, noiseStep + frameProgress), 0, 1, -450, 450)
                                  );
        
        color.setHsb((i * 255 / numberOfActor) % 255, 150, 255, ofMap(k, 0, len, 255, 10)); // Fading transparency from head to tail
        ofSetColor(color);
        ofDrawCircle(location, ofMap(k, len, 0, 3, 12)); // Smaller circles at the end of the trail
      }
    }
    
    canvas->end();
  }
  
  void clear() override {}
  
  int inputCount() override {
    return 1;
  }
  
  ShaderType type() override {
    return ShaderTypeFullHouse;
  }
  
  void drawSettings() override {
    CommonViews::H3Title("FullHouse");
    
    CommonViews::ShaderParameter(settings->speed, settings->speedOscillator);
    CommonViews::ShaderParameter(settings->count, settings->countOscillator);
    CommonViews::ShaderParameter(settings->length, settings->lengthOscillator);
  }
};

#endif
