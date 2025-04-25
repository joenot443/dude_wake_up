//
//  CoreShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef CoreShader_hpp
#define CoreShader_hpp

#include "ofMain.h"
#include "ShaderSettings.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "WaveformOscillator.hpp"
#include "Parameter.hpp"
#include "Shader.hpp"
#include <stdio.h>

struct CoreSettings: public ShaderSettings {
  std::shared_ptr<Parameter> shapeDistance;
  std::shared_ptr<WaveformOscillator> shapeDistanceOscillator;
  
  //Speed
  std::shared_ptr<Parameter> speed;
  std::shared_ptr<WaveformOscillator> speedOscillator;

  // Count
  std::shared_ptr<Parameter> count;
  std::shared_ptr<WaveformOscillator> countOscillator;

  // Color Offsets
  std::shared_ptr<Parameter> colorOffsetR;
  std::shared_ptr<WaveformOscillator> colorOffsetROscillator;
  std::shared_ptr<Parameter> colorOffsetG;
  std::shared_ptr<WaveformOscillator> colorOffsetGOscillator;
  std::shared_ptr<Parameter> colorOffsetB;
  std::shared_ptr<WaveformOscillator> colorOffsetBOscillator;

  // Rate
  std::shared_ptr<Parameter> rate;
  std::shared_ptr<WaveformOscillator> rateOscillator;

  // Rotation Offset
  std::shared_ptr<Parameter> rotationOffset;
  std::shared_ptr<WaveformOscillator> rotationOffsetOscillator;


  CoreSettings(std::string shaderId, json j) :
  shapeDistance(std::make_shared<Parameter>("Shape Distance", 0.5, 0.0, 5.0)),
  shapeDistanceOscillator(std::make_shared<WaveformOscillator>(shapeDistance)),
  speed(std::make_shared<Parameter>("Speed", 0.5, 0.0, 1.0)),
  speedOscillator(std::make_shared<WaveformOscillator>(speed)),
  count(std::make_shared<Parameter>("Count", 20.0, 1.0, 50.0)),
  countOscillator(std::make_shared<WaveformOscillator>(count)),
  colorOffsetR(std::make_shared<Parameter>("Color Offset R", 0.0, -5.0, 5.0)),
  colorOffsetROscillator(std::make_shared<WaveformOscillator>(colorOffsetR)),
  colorOffsetG(std::make_shared<Parameter>("Color Offset G", 1.0, -5.0, 5.0)),
  colorOffsetGOscillator(std::make_shared<WaveformOscillator>(colorOffsetG)),
  colorOffsetB(std::make_shared<Parameter>("Color Offset B", 2.0, -5.0, 5.0)),
  colorOffsetBOscillator(std::make_shared<WaveformOscillator>(colorOffsetB)),
  rate(std::make_shared<Parameter>("Rate", 2.0, 0.0, 10.0)),
  rateOscillator(std::make_shared<WaveformOscillator>(rate)),
  rotationOffset(std::make_shared<Parameter>("Rotation Offset", 0.1, 0.0, 1.0)),
  rotationOffsetOscillator(std::make_shared<WaveformOscillator>(rotationOffset)),
  ShaderSettings(shaderId, j, "Core") {
    parameters = { shapeDistance, speed, count, colorOffsetR, colorOffsetG, colorOffsetB, rate, rotationOffset };
    oscillators = { shapeDistanceOscillator, speedOscillator, countOscillator, colorOffsetROscillator, colorOffsetGOscillator, colorOffsetBOscillator, rateOscillator, rotationOffsetOscillator };
    load(j);
    registerParameters();
  };
};

struct CoreShader: Shader {
  CoreSettings *settings;
  CoreShader(CoreSettings *settings) : settings(settings), Shader(settings) {};

  void setup() override {
    shader.load("shaders/Core");
  }

  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override {
    canvas->begin();
    shader.begin();
    ofSetColor(0, 0, 0, 0);
    ofDrawRectangle(0, 0, frame->getWidth(), frame->getHeight());
//    ofClear(0,0,0, 255);
//    ofClear(0,0,0, 0);
    shader.setUniformTexture("tex", frame->getTexture(), 4);
    shader.setUniform1f("shapeDistance", settings->shapeDistance->value);
    shader.setUniform1f("speed", settings->speed->value);
    shader.setUniform1f("time", ofGetElapsedTimef());
    shader.setUniform2f("dimensions", frame->getWidth(), frame->getHeight());
    shader.setUniform1f("u_count", settings->count->value);
    shader.setUniform1f("u_colorOffsetR", settings->colorOffsetR->value);
    shader.setUniform1f("u_colorOffsetG", settings->colorOffsetG->value);
    shader.setUniform1f("u_colorOffsetB", settings->colorOffsetB->value);
    shader.setUniform1f("u_rate", settings->rate->value);
    shader.setUniform1f("u_rotationOffset", settings->rotationOffset->value);
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
    return ShaderTypeCore;
  }

  void drawSettings() override {
    

    CommonViews::ShaderParameter(settings->shapeDistance, settings->shapeDistanceOscillator);
    CommonViews::ShaderParameter(settings->speed, settings->speedOscillator);
    CommonViews::ShaderParameter(settings->count, settings->countOscillator);
    CommonViews::ShaderParameter(settings->colorOffsetR, settings->colorOffsetROscillator);
    CommonViews::ShaderParameter(settings->colorOffsetG, settings->colorOffsetGOscillator);
    CommonViews::ShaderParameter(settings->colorOffsetB, settings->colorOffsetBOscillator);
    CommonViews::ShaderParameter(settings->rate, settings->rateOscillator);
    CommonViews::ShaderParameter(settings->rotationOffset, settings->rotationOffsetOscillator);
  }
};

#endif
