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

const int FrameBufferCount=30;

// Basic
struct HSBSettings: public ShaderSettings {
  std::string settingsId;
  Parameter hue;
  Parameter saturation;
  Parameter brightness;
  bool invertHue = false;
  bool invertSaturation = false;
  bool invertBrightness = false;
  Oscillator hueOscillator = Oscillator(&hue);
  Oscillator saturationOscillator = Oscillator(&saturation);
  Oscillator brightnessOscillator = Oscillator(&brightness);
  
  HSBSettings(std::string settingsId) :
  settingsId(settingsId),
  hue(Parameter("HSB_hue", settingsId, 1.0, -1.0, 2.0)),
  saturation(Parameter("HSB_saturation", settingsId, 1.0, -1.0, 3.0)),
  brightness(Parameter("HSB_brightness", settingsId, 1.0, -1.0, 3.0)),
  ShaderSettings(settingsId)
  
  {
    parameters = {&hue, &saturation, &brightness};
    oscillators = {&hueOscillator, &saturationOscillator, &brightnessOscillator};
  }
};

struct BlurSettings: public ShaderSettings  {
  std::string settingsId;
  Parameter mix;
  Parameter radius;
  Oscillator mixOscillator;
  Oscillator radiusOscillator;
  BlurSettings(std::string settingsId) :
  mix(Parameter("blur_mix", settingsId, 0.0, 0.0, 1.0)),
  radius(Parameter("blur_radius", settingsId, 1.0, 0.0, 50.0)),
  mixOscillator(&mix),
  radiusOscillator(&radius),
  settingsId(settingsId),
  ShaderSettings(settingsId)  {
    parameters = {&mix, &radius};
    oscillators = {&mixOscillator, &radiusOscillator};
  }
};

struct TransformSettings: public ShaderSettings  {
  std::string settingsId;
  Parameter scale;
  Oscillator scaleOscillator;
  Parameter feedbackBlend;
  Oscillator feedbackBlendOscillator;
  TransformSettings(std::string settingsId) : settingsId(settingsId),
  scale(Parameter("scale", settingsId, "cam1_scale", 1.0, 0.001, 3.0)),
  feedbackBlend(Parameter("feedback_blend", settingsId, "channel1blend", 1.0, 0.0, 1.0)),
  scaleOscillator(Oscillator(&scale)),
  feedbackBlendOscillator(Oscillator(&feedbackBlend)),
  ShaderSettings(settingsId)
  {
    parameters = {&scale};
    oscillators = {&scaleOscillator};
  }
};


struct PixelSettings: public ShaderSettings  {
  Parameter enabled;
  std::string settingsId;
  
  Parameter size;
  Oscillator sizeOscillator;
  
  PixelSettings(std::string settingsId) :
  settingsId(settingsId),
  size(Parameter("pixel_size", settingsId, 24.0,  0.01, 64.0)),
  enabled(Parameter("enabled", settingsId, 0.0,  1.0, 0.0)),
  sizeOscillator(&size),
  ShaderSettings(settingsId)
  {
    parameters = {&size, &enabled};
    oscillators = {&sizeOscillator};
  }
};

// Feedback

struct FeedbackSettings: public ShaderSettings {
  int index;
  Parameter lumaKeyEnabled;
  Parameter useProcessedFrame;
  std::string feedbackId;  
  std::string settingsId;
  
  Parameter feedbackType;
  
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

  
  FeedbackSettings(std::string settingsId, int index) :
  index(index),
  feedbackType(Parameter("feedback_type", settingsId, "fbType", 0.0, 0.0, 2.0)),
  blend(Parameter("feedback_blend", settingsId, "blend", 0.0, 0.0, 1.0)),
  blendOscillator(Oscillator(&blend)),
  mix(Parameter("feedback_mix", settingsId, "fb_mix", 0.0, 0.0, 1.0)),
  mixOscillator(Oscillator(&mix)),
  keyValue(Parameter("feedback_keyValue", settingsId, "lumaKey", 0.0, 0.0, 1.0)),
  keyValueOscillator(Oscillator(&keyValue)),
  keyThreshold(Parameter("feedback_keyThreshold", settingsId, "lumaThresh", 0.0, 0.0, 1.0)),
  keyThresholdOscillator(Oscillator(&keyThreshold)),
  delayAmount(Parameter("feedback_delayAmount", settingsId, 10.0, 0.0, 28.0)),
  delayAmountOscillator(Oscillator(&delayAmount)),
  lumaKeyEnabled(Parameter(formatString("%sfeedback_luma_key_enabled_%d", "lumaEnabled", settingsId.c_str(), index), settingsId, 0.0, 0.0, 0.0)),
  useProcessedFrame(Parameter(formatString("%sfeedback_use_procssed_%d", settingsId.c_str(), index), settingsId, 0.0, 0.0, 0.0)),
  feedbackId(formatString("feedback_%d", index)),
  settingsId(settingsId),
  rotate(Parameter("feedback_rotate", settingsId, 0.0001, 0.0001, TWO_PI)),
  xOffset(Parameter("feedback_xOffset", settingsId, 0.0, -300.0, 300.0)),
  yOffset(Parameter("feedback_yOffset", settingsId, 0.0, -300.0, 300.0)),
  scale(Parameter("feedback_scale", settingsId, 1.0, 0.0001, 3.0)),
  rotationOscillator(Oscillator(&rotate)),
  xOffsetOscillator(Oscillator(&xOffset)),
  yOffsetOscillator(Oscillator(&yOffset)),
  scaleOscillator(Oscillator(&scale)),
  ShaderSettings(settingsId)
  {
    parameters = {&blend, &mix, &keyValue, &keyThreshold, &delayAmount, &rotate, &xOffset, &yOffset, &scale};
    oscillators = {&blendOscillator, &mixOscillator, &keyValueOscillator, &keyThresholdOscillator, &delayAmountOscillator, &rotationOscillator, &xOffsetOscillator, &yOffsetOscillator, &scaleOscillator};
  }
};

enum VideoSource { VideoSource_webcam, VideoSource_file };

struct VideoFlags {
  Parameter resetFeedback;
  std::string settingsId;
  VideoFlags(std::string settingsId) : resetFeedback(Parameter("reset_feedback", settingsId, 0.0, 0.0, 1.0)), settingsId(settingsId) {}
};

struct VideoSettings {
  VideoFlags videoFlags;
  int streamId;
  std::string settingsIdStr;
  
  VideoSettings(int intId, std::string strId) :
  streamId(intId),
  settingsIdStr(strId),
  videoFlags(strId)
  {
  }
};



#endif /* Settings_h */
