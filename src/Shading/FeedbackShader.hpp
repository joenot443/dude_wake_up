//
//  FeedbackShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/5/22.
//

#ifndef FeedbackShader_hpp
#define FeedbackShader_hpp

#include <stdio.h>
#include "ofMain.h"
#include "ShaderConfigSelectionView.hpp"
#include "Shader.hpp"
#include "ofMain.h"
#include "ofxImGui.h"
#include "json.hpp"

using json = nlohmann::json;

// Feedback

struct FeedbackSettings: public ShaderSettings {
  int index;
  std::shared_ptr<Parameter> lumaKeyEnabled;
  std::string shaderId;
  
  std::shared_ptr<Parameter> scale;
  std::shared_ptr<Oscillator> scaleOscillator;
  
  std::shared_ptr<Parameter> xPosition;
  std::shared_ptr<Oscillator> xPositionOscillator;
  
  std::shared_ptr<Parameter> yPosition;
  std::shared_ptr<Oscillator> yPositionOscillator;

  std::shared_ptr<Parameter> blend;
  std::shared_ptr<Oscillator> blendOscillator;
  
  std::shared_ptr<Parameter> mix;
  std::shared_ptr<Oscillator> mixOscillator;
  
  std::shared_ptr<Parameter> delayAmount;
  std::shared_ptr<Oscillator> delayAmountOscillator;
  
  std::shared_ptr<Parameter> keyValue;
  std::shared_ptr<Oscillator> keyValueOscillator;
  
  std::shared_ptr<Parameter> keyThreshold;
  std::shared_ptr<Oscillator> keyThresholdOscillator;
  
  std::shared_ptr<Parameter> sourceSelection;
  
  FeedbackSettings(std::string shaderId, json j) :
  index(index),
  blend(std::make_shared<Parameter>("Blend", 0.5, 0.0, 1.0)),
  blendOscillator(std::make_shared<WaveformOscillator>(blend)),
  keyValue(std::make_shared<Parameter>("Key Value", 0.5, 0.0, 1.0)),
  keyValueOscillator(std::make_shared<WaveformOscillator>(keyValue)),
  keyThreshold(std::make_shared<Parameter>("Key Threshold", 0.5, 0.0, 1.0)),
  keyThresholdOscillator(std::make_shared<WaveformOscillator>(keyThreshold)),
  delayAmount(std::make_shared<Parameter>("Delay Amount", 20.0, 0.0, 28.0)),
  delayAmountOscillator(std::make_shared<WaveformOscillator>(delayAmount)),
  lumaKeyEnabled(std::make_shared<Parameter>("Luma Key Enabled", 0.0, 0.0, 0.0)),
  xPosition(std::make_shared<Parameter>("xPosition", 0.0, 0.0, 1.0)),
  yPosition(std::make_shared<Parameter>("yPosition", 0.5, 0.0, 1.0)),
  xPositionOscillator(std::make_shared<WaveformOscillator>(xPosition)),
  yPositionOscillator(std::make_shared<WaveformOscillator>(yPosition)),
  scale(std::make_shared<Parameter>("Scale", 1.0, 0.0, 2.0)),
  scaleOscillator(std::make_shared<WaveformOscillator>(scale)),
  sourceSelection(std::make_shared<Parameter>("source", 0.0, 0.0, 3.0)),
  shaderId(shaderId),
  ShaderSettings(shaderId, j) {
    parameters = {blend, mix, keyValue, keyThreshold, delayAmount, lumaKeyEnabled, xPosition, yPosition, scale, sourceSelection};
    oscillators = {blendOscillator, mixOscillator, keyValueOscillator, keyThresholdOscillator, delayAmountOscillator, xPositionOscillator, yPositionOscillator, scaleOscillator};

    load(j);
  registerParameters();
  }
  
  void load(json j) override {
    ShaderSettings::load(j);
    registerParameters();
    if (!j.is_object()) {
      return;
    }
  }
  
  json serialize() override {
    json j = ShaderSettings::serialize();
    
    return j;
  }
};

struct FeedbackShader: Shader  {
  FeedbackSettings *settings;
  ofShader shader;
  ofFbo fboFeedback;
  std::shared_ptr<FeedbackSource> feedbackSource;
  
  FeedbackShader(FeedbackSettings *settings) : Shader(settings), settings(settings) {};
  
  void shade();
  void clearFrameBuffer();
  void populateSource();
  void drawFeedbackSourceSelector();
  
  virtual void setup();
  virtual void shade(ofFbo *frame, ofFbo *canvas);
  virtual void drawSettings();
  virtual ShaderType type();
};

#endif /* FeedbackShader_hpp */
