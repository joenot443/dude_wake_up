//
//  Settings.h
//  dude_wake_up
//
//  Created by Joe Crozier on 2022-05-09.
//

#ifndef VideoSettings_h
#define VideoSettings_h

#include "Oscillator.hpp"
#include "Shader.hpp"
#include "Parameter.hpp"
#include "Strings.hpp"
#include "ShaderSettings.hpp"
#include <string>
#include "json.hpp"
#include "macro_scope.hpp"

using json = nlohmann::json;

// Basic
struct HSBSettings: public ShaderSettings {
  std::string shaderId;
  Parameter hue;
  Parameter saturation;
  Parameter brightness;
  Oscillator hueOscillator = Oscillator(&hue);
  Oscillator saturationOscillator = Oscillator(&saturation);
  Oscillator brightnessOscillator = Oscillator(&brightness);
  
  HSBSettings(std::string shaderId, json j) :
  shaderId(shaderId),
  hue(Parameter("HSB_hue", shaderId, 1.0, -1.0, 2.0)),
  saturation(Parameter("HSB_saturation", shaderId, 1.0, -1.0, 3.0)),
  brightness(Parameter("HSB_brightness", shaderId, 1.0, -1.0, 3.0)),
  hueOscillator(&hue),
  saturationOscillator(&saturation),
  brightnessOscillator(&brightness),
  ShaderSettings(shaderId)
  {
    parameters = {&hue, &saturation, &brightness};
    oscillators = {&hueOscillator, &saturationOscillator, &brightnessOscillator};
    load(j);
  }
};

struct BlurSettings: public ShaderSettings  {
  std::string shaderId;
  Parameter mix;
  Parameter radius;
  Oscillator mixOscillator;
  Oscillator radiusOscillator;
  BlurSettings(std::string shaderId, json j) :
  mix(Parameter("blur_mix", shaderId, 0.0, 0.0, 1.0)),
  radius(Parameter("blur_radius", shaderId, 1.0, 0.0, 50.0)),
  mixOscillator(&mix),
  radiusOscillator(&radius),
  shaderId(shaderId),
  ShaderSettings(shaderId)  {
    parameters = {&mix, &radius};
    oscillators = {&mixOscillator, &radiusOscillator};
    load(j);
  }
};

// Pixelate

struct PixelSettings: public ShaderSettings  {
  Parameter enabled;
  std::string shaderId;
  
  Parameter size;
  Oscillator sizeOscillator;
  
  PixelSettings(std::string shaderId, json j) :
  shaderId(shaderId),
  size(Parameter("pixel_size", shaderId, 24.0,  0.01, 64.0)),
  enabled(Parameter("enabled", shaderId, 0.0,  1.0, 0.0)),
  sizeOscillator(&size),
  ShaderSettings(shaderId)
  {
    parameters = {&size, &enabled};
    oscillators = {&sizeOscillator};
    load(j);
  }
};

// Mirror

struct MirrorSettings: public ShaderSettings  {
  std::string shaderId;
  
  Parameter mirrorXEnabled;
  Parameter mirrorYEnabled;
  
  Parameter xOffset;
  Oscillator xOffsetOscillator;
  
  Parameter yOffset;
  Oscillator yOffsetOscillator;
  
  Parameter lockXY;

  MirrorSettings(std::string shaderId, json j) :
  shaderId(shaderId),
  lockXY(Parameter("lockXY", shaderId, 0.0, 1.0, 1.0)),
  xOffset(Parameter("xOffset", shaderId, 0.15,  0.01, 1.0)),
  yOffset(Parameter("yOffset", shaderId, 0.15,  0.01, 1.0)),
  mirrorXEnabled(Parameter("mirrorXEnabled", shaderId, 0.0,  1.0, 0.0)),
  mirrorYEnabled(Parameter("mirrorXEnabled", shaderId, 0.0,  1.0, 0.0)),
  xOffsetOscillator(&xOffset),
  yOffsetOscillator(&yOffset),
  ShaderSettings(shaderId)
  {
    parameters = {&lockXY, &xOffset, &yOffset, &mirrorXEnabled, &mirrorYEnabled};
    oscillators = {&xOffsetOscillator, &yOffsetOscillator};
    load(j);
  }
};

// Transform

struct TransformSettings: public ShaderSettings {
  std::string shaderId;
  
  Parameter xTranslate;
  Oscillator xTranslateOscillator;
  Parameter yTranslate;
  Oscillator yTranslateOscillator;

  Parameter xScale;
  Oscillator xScaleOscillator;

  Parameter yScale;
  Oscillator yScaleOscillator;

  Parameter rotate;
  Oscillator rotateOscillator;

  TransformSettings(std::string shaderId, json j) :
  shaderId(shaderId),
  xTranslate(Parameter("xTranslate", shaderId, 0.0,  -1.0, 1.0)),
  yTranslate(Parameter("yTranslate", shaderId, 0.0,  -1.0, 1.0)),
  xScale(Parameter("xScale", shaderId, 1.0,  0.0, 2.0)),
  yScale(Parameter("yScale", shaderId, 1.0,  0.0, 2.0)),
  rotate(Parameter("rotate", shaderId, 0.0,  0.0, 360.0)),
  xTranslateOscillator(&xTranslate),
  yTranslateOscillator(&yTranslate),
  xScaleOscillator(&xScale),
  yScaleOscillator(&yScale),
  rotateOscillator(&rotate),
  ShaderSettings(shaderId)
  {
    parameters = {&xTranslate, &yTranslate, &xScale, &yScale, &rotate};
    oscillators = {&xTranslateOscillator, &yTranslateOscillator, &xScaleOscillator, &yScaleOscillator, &rotateOscillator};
    load(j);
  }
};

// Feedback

struct FeedbackSettings: public ShaderSettings {
  int index;
  Parameter lumaKeyEnabled;
  Parameter useProcessedFrame;
  std::string shaderId;
  std::string feedbackSourceId;
  std::string chainerId;
  
  Parameter blend;
  Oscillator blendOscillator;
  
  Parameter mix;
  Oscillator mixOscillator;
  
  Parameter delayAmount;
  Oscillator delayAmountOscillator;
  
  Parameter keyValue;
  Oscillator keyValueOscillator;
  
  Parameter keyThreshold;
  Oscillator keyThresholdOscillator;
  
  Parameter rotate;
  Oscillator rotationOscillator;

  Parameter xOffset;
  Oscillator xOffsetOscillator;

  Parameter yOffset;
  Oscillator yOffsetOscillator;

  Parameter scale;
  Oscillator scaleOscillator;

  
  FeedbackSettings(std::string shaderId, std::string chainerId, json j) :
  index(index),
  feedbackSourceId(chainerId),
  chainerId(chainerId),
  blend(Parameter("feedback_blend", shaderId, "blend", 0.0, 0.0, 1.0)),
  blendOscillator(Oscillator(&blend)),
  mix(Parameter("feedback_mix", shaderId, "fb_mix", 0.0, 0.0, 1.0)),
  mixOscillator(Oscillator(&mix)),
  keyValue(Parameter("feedback_keyValue", shaderId, "lumaKey", 0.0, 0.0, 1.0)),
  keyValueOscillator(Oscillator(&keyValue)),
  keyThreshold(Parameter("feedback_keyThreshold", shaderId, "lumaThresh", 0.0, 0.0, 1.0)),
  keyThresholdOscillator(Oscillator(&keyThreshold)),
  delayAmount(Parameter("feedback_delayAmount", shaderId, 10.0, 0.0, 28.0)),
  delayAmountOscillator(Oscillator(&delayAmount)),
  lumaKeyEnabled(Parameter("feedback_luma_key_enabled", shaderId, "lumaEnabled", 0.0, 0.0, 0.0)),
  useProcessedFrame(Parameter("feedback_use_procssed", shaderId, 0.0, 0.0, 0.0)),
  shaderId(shaderId),
  rotate(Parameter("feedback_rotate", shaderId, 0.0001, 0.0001, TWO_PI)),
  xOffset(Parameter("feedback_xOffset", shaderId, 0.0, -300.0, 300.0)),
  yOffset(Parameter("feedback_yOffset", shaderId, 0.0, -300.0, 300.0)),
  scale(Parameter("feedback_scale", shaderId, 1.0, 0.0001, 3.0)),
  rotationOscillator(Oscillator(&rotate)),
  xOffsetOscillator(Oscillator(&xOffset)),
  yOffsetOscillator(Oscillator(&yOffset)),
  scaleOscillator(Oscillator(&scale)),
  ShaderSettings(shaderId) {
    parameters = {&blend, &mix, &keyValue, &keyThreshold, &delayAmount, &lumaKeyEnabled, &useProcessedFrame, &rotate, &xOffset, &yOffset, &scale};
    oscillators = {&blendOscillator, &mixOscillator, &keyValueOscillator, &keyThresholdOscillator, &delayAmountOscillator, &rotationOscillator, &xOffsetOscillator, &yOffsetOscillator, &scaleOscillator};
    load(j);
  }
};


struct VideoFlags {
  Parameter resetFeedback;
  std::string shaderId;
  VideoFlags(std::string shaderId) : resetFeedback(Parameter("reset_feedback", shaderId, 0.0, 0.0, 1.0)), shaderId(shaderId) {}
};

struct VideoSettings {
  VideoFlags videoFlags;
  int streamId;
  std::string shaderIdStr;
  
  VideoSettings(int intId, std::string strId) :
  streamId(intId),
  shaderIdStr(strId),
  videoFlags(strId)
  {
  }
};



#endif /* Settings_h */
