//
//  CirclePathShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef CirclePathShader_hpp
#define CirclePathShader_hpp

#include "ofMain.h"
#include "ShaderSettings.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "WaveformOscillator.hpp"
#include "Parameter.hpp"
#include "Shader.hpp"
#include <stdio.h>

struct CirclePathSettings : public ShaderSettings
{
  std::shared_ptr<Parameter> radius;
  std::shared_ptr<Parameter> scale;
  std::shared_ptr<Parameter> speed;

  std::shared_ptr<WaveformOscillator> radiusOscillator;
  std::shared_ptr<WaveformOscillator> scaleOscillator;
  std::shared_ptr<WaveformOscillator> speedOscillator;

  CirclePathSettings(std::string shaderId, json j, std::string name) : radius(std::make_shared<Parameter>("radius", 0.4, 0.1, 1.0)),
                                                     scale(std::make_shared<Parameter>("scale", 1.0, 0.1, 10.0)),
                                                     speed(std::make_shared<Parameter>("speed", 0.05, 0.01, 0.2)), // Default speed and range, adjust as needed
                                                     ShaderSettings(shaderId, j, name)
  {
    radiusOscillator = std::make_shared<WaveformOscillator>(radius);
    scaleOscillator = std::make_shared<WaveformOscillator>(scale);
    speedOscillator = std::make_shared<WaveformOscillator>(speed);

    parameters = {radius, scale, speed};
    oscillators = {radiusOscillator, scaleOscillator, speedOscillator};
    load(j);
    registerParameters();
  };
};

struct CirclePathShader : Shader
{
  CirclePathSettings *settings;
  float angle;
  glm::vec2 center;

  CirclePathShader(CirclePathSettings *settings) : settings(settings), Shader(settings), angle(0.0) {};
  

  void setup() override
  {
    // Setup your shader as needed
  }

  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override
  {
    canvas->begin();
    // Clear the frame
    ofClear(0,0,0, 255);
    ofClear(0,0,0, 0);

    // Dynamically adjust radius, scale, and speed using the 'value' member
    float adjustedRadius = std::min(canvas->getWidth(), canvas->getHeight()) * settings->radius->value;
    float scale = settings->scale->value;
    float speed = settings->speed->value; // Use the dynamic speed value

    center = glm::vec2(canvas->getWidth() * 0.5, canvas->getHeight() * 0.5);

    glm::vec2 position;
    position.x = center.x + adjustedRadius * cos(angle);
    position.y = center.y + adjustedRadius * sin(angle);

    angle += speed; // Use dynamic speed for angle increment

    // Apply scale to the frame's dimensions when drawing
    frame->draw(position.x - (frame->getWidth() * scale * 0.5), position.y - (frame->getHeight() * scale * 0.5), frame->getWidth() * scale, frame->getHeight() * scale);

    canvas->end();
  }

  void clear() override
  {
    // Implement any necessary cleanup
  }

    int inputCount() override {
    return 1;
  }
ShaderType type() override { 
    return ShaderTypeCirclePath;
  }

  void drawSettings() override
  {
    CommonViews::H3Title("Circle Path");
    CommonViews::ShaderParameter(settings->speed, settings->speedOscillator);
    CommonViews::ShaderParameter(settings->radius, settings->radiusOscillator);
    CommonViews::ShaderParameter(settings->scale, settings->scaleOscillator);
    // Add settings UI here, if applicable
  }
};

#endif
