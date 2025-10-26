//
//  StageShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef StageShader_hpp
#define StageShader_hpp

#include "ofMain.h"
#include "ShaderSettings.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "TimeService.hpp"
#include "WaveformOscillator.hpp"
#include "Parameter.hpp"
#include "Shader.hpp"
#include <stdio.h>

struct StageSettings: public ShaderSettings {
  std::shared_ptr<Parameter> positionX;
  std::shared_ptr<Parameter> positionY;
  std::shared_ptr<Parameter> scale;
  std::shared_ptr<Parameter> rotation;
  std::shared_ptr<Parameter> alpha;

  std::shared_ptr<WaveformOscillator> positionXOscillator;
  std::shared_ptr<WaveformOscillator> positionYOscillator;
  std::shared_ptr<WaveformOscillator> scaleOscillator;
  std::shared_ptr<WaveformOscillator> rotationOscillator;
  std::shared_ptr<WaveformOscillator> alphaOscillator;

  StageSettings(std::string shaderId, json j) :
  positionX(std::make_shared<Parameter>("X", 0.5, 0.0, 1.0)),
  positionY(std::make_shared<Parameter>("Y", 0.5, 0.0, 1.0)),
  scale(std::make_shared<Parameter>("Scale", 1.0, 0.0, 5.0)),
  rotation(std::make_shared<Parameter>("Rotation", 0.0, -TWO_PI, TWO_PI)),
  alpha(std::make_shared<Parameter>("Alpha", 1.0, 0.0, 1.0)),
  positionXOscillator(std::make_shared<WaveformOscillator>(positionX)),
  positionYOscillator(std::make_shared<WaveformOscillator>(positionY)),
  scaleOscillator(std::make_shared<WaveformOscillator>(scale)),
  rotationOscillator(std::make_shared<WaveformOscillator>(rotation)),
  alphaOscillator(std::make_shared<WaveformOscillator>(alpha)),
  ShaderSettings(shaderId, j, "Stage") {
    parameters = { positionX, positionY, scale, rotation, alpha };
    oscillators = { positionXOscillator, positionYOscillator, scaleOscillator, rotationOscillator, alphaOscillator };
    audioReactiveParameter = scale;
    load(j);
    registerParameters();
  };
};

struct StageShader: Shader {
  StageSettings *settings;
  StageShader(StageSettings *settings) : settings(settings), Shader(settings) {};
  
  void setup() override {
    // No shader needed - using direct draw calls
  }

  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override {
    canvas->begin();
    ofClear(0, 0, 0, 0);

    // Draw the background (main input) full-screen
    frame->draw(0, 0, canvas->getWidth(), canvas->getHeight());

    // Check if we have a foreground input
    if (hasInputAtSlot(InputSlotTwo)) {
      std::shared_ptr<ofFbo> foregroundFrame = inputAtSlot(InputSlotTwo)->frame();

      if (foregroundFrame && foregroundFrame->isAllocated()) {
        ofPushMatrix();
        ofPushStyle();

        // Calculate position in canvas space
        float x = settings->positionX->value * canvas->getWidth();
        float y = settings->positionY->value * canvas->getHeight();

        // Get foreground dimensions
        float fgWidth = foregroundFrame->getWidth() * settings->scale->value;
        float fgHeight = foregroundFrame->getHeight() * settings->scale->value;

        // Translate to position (centered on the position point)
        ofTranslate(x, y);

        // Apply rotation if needed
        if (settings->rotation->value != 0.0) {
          ofRotateRad(settings->rotation->value);
        }

        // Set alpha
        ofSetColor(255, 255, 255, settings->alpha->value * 255);

        // Draw centered on the position point
        foregroundFrame->draw(-fgWidth / 2.0, -fgHeight / 2.0, fgWidth, fgHeight);

        ofPopStyle();
        ofPopMatrix();
      }
    }

    canvas->end();
  }

  void clear() override {

  }

  int inputCount() override {
    return 2;
  }

  ShaderType type() override {
    return ShaderTypeStage;
  }

  void drawSettings() override {
    CommonViews::H3Title("Stage");

    CommonViews::MultiSlider("Position", settings->shaderId,
                            settings->positionX, settings->positionY,
                            settings->positionXOscillator, settings->positionYOscillator,
                            0.5625);

    CommonViews::ShaderParameter(settings->scale, settings->scaleOscillator);
    CommonViews::ShaderParameter(settings->rotation, settings->rotationOscillator);
    CommonViews::ShaderParameter(settings->alpha, settings->alphaOscillator);
  }
};

#endif /* StageShader_hpp */
