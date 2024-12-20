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
  
  std::shared_ptr<Parameter> mainAlpha;
  std::shared_ptr<Oscillator> mainAlphaOscillator;
  
  std::shared_ptr<Parameter> feedbackAlpha;
  std::shared_ptr<Oscillator> feedbackAlphaOscillator;
  
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
  
  std::shared_ptr<Parameter> shouldClearFeedbackBuffer;
  
  FeedbackSettings(std::string shaderId, json j, std::string name) : index(index),
  priority(std::make_shared<Parameter>("Main Takes Priority", ParameterType_Bool)),
  blendMode(std::make_shared<Parameter>("Blend Mode", 4.0, 0.0, 15.0, ParameterType_Int)),
  mainAlpha(std::make_shared<Parameter>("Main Alpha", 1.0, 0.0, 1.0)),
  mainAlphaOscillator(std::make_shared<WaveformOscillator>(mainAlpha)),
  feedbackAlpha(std::make_shared<Parameter>("Feedback Alpha", 0.5, 0.0, 1.0)),
  feedbackAlphaOscillator(std::make_shared<WaveformOscillator>(feedbackAlpha)),
  feedbackMix(std::make_shared<Parameter>("Feedback Mix", 0.6, 0.0, 1.0)),
  feedbackMixOscillator(std::make_shared<WaveformOscillator>(feedbackMix)),
  rotation(std::make_shared<Parameter>("Rotation", 0.0, -PI / 2.0, PI / 2.0)),
  rotationOscillator(std::make_shared<WaveformOscillator>(rotation)),
  
  keyValue(std::make_shared<Parameter>("Key Value", 0.5, 0.0, 1.0)),
  keyValueOscillator(std::make_shared<WaveformOscillator>(keyValue)),
  keyThreshold(std::make_shared<Parameter>("Key Threshold", 0.5, 0.0, 1.0)),
  keyThresholdOscillator(std::make_shared<WaveformOscillator>(keyThreshold)),
  delayAmount(std::make_shared<Parameter>("Delay Amount", 20.0, 0.0, 28.0)),
  delayAmountOscillator(std::make_shared<WaveformOscillator>(delayAmount)),
  lumaKeyEnabled(std::make_shared<Parameter>("Luma Key Enabled", 0.0, 0.0, 0.0)),
  xPosition(std::make_shared<Parameter>("X Position", 0.0, -1.0, 1.0)),
  yPosition(std::make_shared<Parameter>("Y Position", 0.0, -1.0, 1.0)),
  xPositionOscillator(std::make_shared<WaveformOscillator>(xPosition)),
  yPositionOscillator(std::make_shared<WaveformOscillator>(yPosition)),
  scale(std::make_shared<Parameter>("Scale", 1.0, 0.0, 2.0)),
  scaleOscillator(std::make_shared<WaveformOscillator>(scale)),
  sourceSelection(std::make_shared<Parameter>("Source", 1.0, 0.0, 3.0)),
  shouldClearFeedbackBuffer(std::make_shared<Parameter>("Clear Feedback Buffer", ParameterType_Bool)),
  shaderId(shaderId),
  ShaderSettings(shaderId, j, name)
  {
    parameters = {mainAlpha, feedbackMix, feedbackAlpha, keyValue, keyThreshold, delayAmount, lumaKeyEnabled, xPosition, yPosition, scale, sourceSelection, blendMode, priority, rotation, shouldClearFeedbackBuffer};
    oscillators = {mainAlphaOscillator, feedbackMixOscillator, feedbackAlphaOscillator, keyValueOscillator, keyThresholdOscillator, delayAmountOscillator, xPositionOscillator, yPositionOscillator, scaleOscillator, rotationOscillator };
    
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
  
  std::vector<std::string> sourceNames = {
    "Origin Source",
    "Feedback",
    "Final Node",
    "Original",
  };
  
  FeedbackShader(FeedbackSettings *settings) : Shader(settings),
  settings(settings){};
  
  void shade();
  void clearFrameBuffer();
  void populateSource();
  ofTexture feedbackTexture();
  void drawFeedbackSourceSelector();
  void clearFrameIfNeeded();
  int frameIndex();
  int inputCount() override;
  
  void drawPreview(ImVec2 pos, float scale) override;
  void setup() override;
  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override;
  void drawSettings() override;
  ShaderType type() override;
};

#endif
