//
//  BounceShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef BounceShader_hpp
#define BounceShader_hpp

#include "ofMain.h"
#include "ShaderSettings.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "ValueOscillator.hpp"
#include "Parameter.hpp"
#include "Shader.hpp"
#include <stdio.h>

struct BounceSettings: public ShaderSettings {
  std::shared_ptr<Parameter> speed;
  std::shared_ptr<ValueOscillator> speedOscillator;
  
  std::shared_ptr<Parameter> scale;
  std::shared_ptr<ValueOscillator> scaleOscillator;
  
  BounceSettings(std::string shaderId, json j) :
  speed(std::make_shared<Parameter>("speed", 1.0, 0.0, 10.0)),
  scale(std::make_shared<Parameter>("scale", 0.1, 0.0, 1.0)),
  speedOscillator(std::make_shared<ValueOscillator>(speed)),
  scaleOscillator(std::make_shared<ValueOscillator>(scale)),
  ShaderSettings(shaderId, j) {
    parameters = { speed, scale };
    oscillators = { speedOscillator, scaleOscillator };
    load(j);
    registerParameters();
  };
};


struct BounceShader: Shader {
  BounceSettings *settings;
  glm::vec2 position;
  glm::vec2 velocity;
  
  BounceShader(BounceSettings *settings) : settings(settings), Shader(settings), position(0, 0) {
    // Initialize velocity with a static speed in the down-right direction
    velocity = glm::vec2(1, 1); // You can adjust the values for different speeds/directions
  };
  
  void setup() override {
    // Setup your shader as needed
  }
  
  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override {
    canvas->begin();
    ofClear(0,0,0, 255);
    ofClear(0,0,0, 0);
    
    // Update position based on velocity
    position.x += velocity.x * settings->speed->value;
    position.y += velocity.y * settings->speed->value;
    
    // Check for collisions with the canvas edges
    if (position.x <= 0 || position.x >= canvas->getWidth() - frame->getWidth() * settings->scale->value) {
      velocity.x *= -1; // Reverse X direction
    }
    if (position.y <= 0 || position.y >= canvas->getHeight() - frame->getHeight() * settings->scale->value) {
      velocity.y *= -1; // Reverse Y direction
    }
    
    // Draw the frame at the new position, scaled
    frame->draw(position.x, position.y, frame->getWidth() * settings->scale->value, frame->getHeight() * settings->scale->value);
    
    canvas->end();
  }
  
  void clear() override {
    // Implement any necessary cleanup
  }
  
  ShaderType type() override {
    return ShaderTypeBounce;
  }
  
  void drawSettings() override {
    CommonViews::H3Title("Bounce");
    CommonViews::ShaderParameter(settings->speed, settings->speedOscillator);
    CommonViews::ShaderParameter(settings->scale, settings->speedOscillator);
    
    if (ImGui::Button("Reset")) {
      position = ImVec2(0, 0);
    }
  }
};

#endif /* BounceShader_hpp */