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

struct FeedbackSettings : public ShaderSettings
{
public:
  int index;
  std::shared_ptr<Parameter> lumaKeyEnabled;
  std::string shaderId;
  
  std::shared_ptr<Parameter> priority;
  
  std::shared_ptr<Parameter> blendMode;
  
  std::shared_ptr<Parameter> scale;
  std::shared_ptr<Oscillator> scaleOscillator;
  
  std::shared_ptr<Parameter> xPosition;
  std::shared_ptr<Oscillator> xPositionOscillator;
  
  std::shared_ptr<Parameter> yPosition;
  std::shared_ptr<Oscillator> yPositionOscillator;
  
  std::shared_ptr<Parameter> mainMix;
  std::shared_ptr<Oscillator> mainMixOscillator;
  
  std::shared_ptr<Parameter> feedbackMix;
  std::shared_ptr<Oscillator> feedbackMixOscillator;
  
  std::shared_ptr<Parameter> delayAmount;
  std::shared_ptr<Oscillator> delayAmountOscillator;
  
  std::shared_ptr<Parameter> keyValue;
  std::shared_ptr<Oscillator> keyValueOscillator;
  
  std::shared_ptr<Parameter> keyThreshold;
  std::shared_ptr<Oscillator> keyThresholdOscillator;
  
  std::shared_ptr<Parameter> rotation;
  std::shared_ptr<Oscillator> rotationOscillator;
  
  std::shared_ptr<Parameter> sourceSelection;
  
  FeedbackSettings(std::string shaderId, json j, std::string name) : index(index),
  priority(std::make_shared<Parameter>("priority", 0.0, 0.0, 1.0)),
  blendMode(std::make_shared<Parameter>("Blend Mode", 0.0, 0.0, 15.0)),
  mainMix(std::make_shared<Parameter>("Main Mix", 0.5, 0.0, 1.0)),
  mainMixOscillator(std::make_shared<WaveformOscillator>(mainMix)),
  feedbackMix(std::make_shared<Parameter>("Feedback Mix", 0.95, 0.0, 1.0)),
  feedbackMixOscillator(std::make_shared<WaveformOscillator>(feedbackMix)),
  rotation(std::make_shared<Parameter>("Rotation", 0.0, 0.0, TWO_PI)),
  rotationOscillator(std::make_shared<WaveformOscillator>(rotation)),
  
  keyValue(std::make_shared<Parameter>("Key Value", 0.5, 0.0, 1.0)),
  keyValueOscillator(std::make_shared<WaveformOscillator>(keyValue)),
  keyThreshold(std::make_shared<Parameter>("Key Threshold", 0.5, 0.0, 1.0)),
  keyThresholdOscillator(std::make_shared<WaveformOscillator>(keyThreshold)),
  delayAmount(std::make_shared<Parameter>("Delay Amount", 20.0, 0.0, 28.0)),
  delayAmountOscillator(std::make_shared<WaveformOscillator>(delayAmount)),
  lumaKeyEnabled(std::make_shared<Parameter>("Luma Key Enabled", 0.0, 0.0, 0.0)),
  xPosition(std::make_shared<Parameter>("xPosition", 0.0, -1.0, 1.0)),
  yPosition(std::make_shared<Parameter>("yPosition", 0.0, -1.0, 1.0)),
  xPositionOscillator(std::make_shared<WaveformOscillator>(xPosition)),
  yPositionOscillator(std::make_shared<WaveformOscillator>(yPosition)),
  scale(std::make_shared<Parameter>("Scale", 1.0, 0.0, 2.0)),
  scaleOscillator(std::make_shared<WaveformOscillator>(scale)),
  sourceSelection(std::make_shared<Parameter>("source", 1.0, 0.0, 3.0)),
  shaderId(shaderId),
  ShaderSettings(shaderId, j, name)
  {
    parameters = {mainMix, feedbackMix, keyValue, keyThreshold, delayAmount, lumaKeyEnabled, xPosition, yPosition, scale, sourceSelection, blendMode};
    oscillators = {mainMixOscillator, feedbackMixOscillator, keyValueOscillator, keyThresholdOscillator, delayAmountOscillator, xPositionOscillator, yPositionOscillator, scaleOscillator};
    
    load(j);
    registerParameters();
  }
  
  void load(json j) override
  {
    ShaderSettings::load(j);
    registerParameters();
  }
};

struct FeedbackShader : Shader
{
  FeedbackSettings *settings;
  ofShader shader;
  ofFbo fboFeedback;
  std::shared_ptr<FeedbackSource> feedbackSource;
  
  std::vector<std::string> blendModeNames = {
    "Mix",
    "Multiply",
    "Screen",
    "Darken",
    "Lighten",
    "Difference",
    "Exclusion",
    "Overlay",
    "Hard Light",
    "Soft Light",
    "Color Dodge",
    "Linear Dodge",
    "Burn",
    "Linear Burn"
  };
  
  FeedbackShader(FeedbackSettings *settings) : Shader(settings),
  settings(settings){};
  
  void shade();
  void clearFrameBuffer();
  void populateSource();
  void drawFbo(std::shared_ptr<ofFbo> fbo);
  void drawFeedbackSourceSelector();
  int frameIndex();
  int inputCount() override;
  
  void drawPreview(ImVec2 pos, float scale) override;
  void setup() override;
  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override;
  void drawSettings() override;
  ShaderType type() override;
};

#endif /* FeedbackShader_hpp */
