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
  std::string feedbackSourceId = "";

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

  
  FeedbackSettings(std::string shaderId, json j) :
  index(index),
  blend(std::make_shared<Parameter>("Blend", shaderId, "blend", 0.5, 0.0, 1.0)),
  blendOscillator(std::make_shared<WaveformOscillator>(blend)),
  keyValue(std::make_shared<Parameter>("Key Value", shaderId, "lumaKey", 0.0, 0.0, 1.0)),
  keyValueOscillator(std::make_shared<WaveformOscillator>(keyValue)),
  keyThreshold(std::make_shared<Parameter>("Key Threshold", shaderId, "lumaThresh", 0.0, 0.0, 1.0)),
  keyThresholdOscillator(std::make_shared<WaveformOscillator>(keyThreshold)),
  delayAmount(std::make_shared<Parameter>("Delay Amount", shaderId, 20.0, 0.0, 28.0)),
  delayAmountOscillator(std::make_shared<WaveformOscillator>(delayAmount)),
  lumaKeyEnabled(std::make_shared<Parameter>("Luma Key Enabled", shaderId, "lumaEnabled", 0.0, 0.0, 0.0)),
  shaderId(shaderId),
  ShaderSettings(shaderId) {
    parameters = {blend, mix, keyValue, keyThreshold, delayAmount, lumaKeyEnabled};
    oscillators = {blendOscillator, mixOscillator, keyValueOscillator, keyThresholdOscillator, delayAmountOscillator};

    load(j);
  }
  
  void load(json j) override {
    ShaderSettings::load(j);
    if (!j.is_object()) {
      return;
    }
    
    feedbackSourceId = j["feedbackSourceId"];
  }
  
  json serialize() override {
    json j = ShaderSettings::serialize();
    j["feedbackSourceId"] = feedbackSourceId.c_str();
    
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
  void drawFeedbackSourceSelector();
  
  virtual void setup();
  virtual void shade(ofFbo *frame, ofFbo *canvas);
  virtual void drawSettings();
  virtual ShaderType type();
};

#endif /* FeedbackShader_hpp */
