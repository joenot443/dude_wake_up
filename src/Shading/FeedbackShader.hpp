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
#include "VideoSettings.hpp"
#include "ofMain.h"
#include "ofxImGui.h"

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
  
  std::shared_ptr<Parameter> rotate;
  std::shared_ptr<Oscillator> rotationOscillator;

  std::shared_ptr<Parameter> xOffset;
  std::shared_ptr<Oscillator> xOffsetOscillator;

  std::shared_ptr<Parameter> yOffset;
  std::shared_ptr<Oscillator> yOffsetOscillator;

  std::shared_ptr<Parameter> scale;
  std::shared_ptr<Oscillator> scaleOscillator;

  
  FeedbackSettings(std::string shaderId, json j) :
  index(index),
  blend(std::make_shared<Parameter>("feedback_blend", shaderId, "blend", 0.0, 0.0, 1.0)),
  blendOscillator(std::make_shared<Oscillator>(blend)),
  mix(std::make_shared<Parameter>("feedback_mix", shaderId, "fb_mix", 0.0, 0.0, 1.0)),
  mixOscillator(std::make_shared<Oscillator>(mix)),
  keyValue(std::make_shared<Parameter>("feedback_keyValue", shaderId, "lumaKey", 0.0, 0.0, 1.0)),
  keyValueOscillator(std::make_shared<Oscillator>(keyValue)),
  keyThreshold(std::make_shared<Parameter>("feedback_keyThreshold", shaderId, "lumaThresh", 0.0, 0.0, 1.0)),
  keyThresholdOscillator(std::make_shared<Oscillator>(keyThreshold)),
  delayAmount(std::make_shared<Parameter>("feedback_delayAmount", shaderId, 10.0, 0.0, 28.0)),
  delayAmountOscillator(std::make_shared<Oscillator>(delayAmount)),
  lumaKeyEnabled(std::make_shared<Parameter>("feedback_luma_key_enabled", shaderId, "lumaEnabled", 0.0, 0.0, 0.0)),
  shaderId(shaderId),
  rotate(std::make_shared<Parameter>("feedback_rotate", shaderId, 0.0001, 0.0001, TWO_PI)),
  xOffset(std::make_shared<Parameter>("feedback_xOffset", shaderId, 0.0, -300.0, 300.0)),
  yOffset(std::make_shared<Parameter>("feedback_yOffset", shaderId, 0.0, -300.0, 300.0)),
  scale(std::make_shared<Parameter>("feedback_scale", shaderId, 1.0, 0.0001, 3.0)),
  rotationOscillator(std::make_shared<Oscillator>(rotate)),
  xOffsetOscillator(std::make_shared<Oscillator>(xOffset)),
  yOffsetOscillator(std::make_shared<Oscillator>(yOffset)),
  scaleOscillator(std::make_shared<Oscillator>(scale)),
  ShaderSettings(shaderId) {
    parameters = {blend, mix, keyValue, keyThreshold, delayAmount, lumaKeyEnabled, rotate, xOffset, yOffset, scale};
    oscillators = {blendOscillator, mixOscillator, keyValueOscillator, keyThresholdOscillator, delayAmountOscillator, rotationOscillator, xOffsetOscillator, yOffsetOscillator, scaleOscillator};
    load(j);
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
  void disableFeedback();
  void drawFeedbackSourceSelector();
  
  virtual void setup();
  virtual void shade(ofFbo *frame, ofFbo *canvas);
  virtual void drawSettings();
  virtual ShaderType type();
};

#endif /* FeedbackShader_hpp */
