//
//  Settings.h
//  dude_wake_up
//
//  Created by Joe Crozier on 2022-05-09.
//

#ifndef VideoSettings_h
#define VideoSettings_h

#include "Oscillator.hpp"
#include "Parameter.h"
#include "Strings.hpp"
#include <string>
#include "json.hpp"
#include "macro_scope.hpp"

using json = nlohmann::json;

const int FrameBufferCount=30;

// Basic

class JSONSerializable {
public:
  std::vector<Parameter*> parameters;
  std::vector<Oscillator*> oscillators;
  json serialize(json j) {
    for (auto p : parameters) {
      j[p->paramId] = p->value;
    }
    auto ops = Oscillator::parametersFromOscillators(oscillators);
    for (auto p : ops) {
      j[p->paramId] = p->value;
    }
    return j;
  }
};

struct HSBSettings: public JSONSerializable {
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
  hue(Parameter("HSB_hue", settingsId, 1.0, 0.0, 2.0)),
  saturation(Parameter("HSB_saturation", settingsId, 1.0, 0.0, 3.0)),
  brightness(Parameter("HSB_brightness", settingsId, 1.0, 0.0, 3.0))
  
  {
    parameters = {&hue, &saturation, &brightness};
    oscillators = {&hueOscillator, &saturationOscillator, &brightnessOscillator};
  }
};

struct BlurSettings: public JSONSerializable  {
  std::string settingsId;
  Parameter amount = Parameter("blur_amount", settingsId, 0.0, 0.0, 1.0);
  Parameter radius = Parameter("blur_radius", settingsId, 1.0, 0.0, 50.0);
  Oscillator amountOscillator = Oscillator(&amount);
  Oscillator radiusOscillator = Oscillator(&radius);
  BlurSettings(std::string settingsId) :
  settingsId(settingsId)
  {
    parameters = {&amount, &radius};
    oscillators = {&amountOscillator, &radiusOscillator};
  }
};

struct TransformSettings: public JSONSerializable  {
  std::string settingsId;
  Parameter scale = Parameter("scale", settingsId, "cam1_scale", 1.0, 0.001, 3.0);
  Oscillator scaleOscillator = Oscillator(&scale);
  TransformSettings(std::string settingsId) : settingsId(settingsId) {
    parameters = {&scale};
    oscillators = {&scaleOscillator};
  }
};

struct SharpenSettings: public JSONSerializable  {
  std::string settingsId;
  Parameter amount = Parameter("sharpen_amount", settingsId, 0.0, 0.0, 2.0);
  Parameter radius = Parameter("sharpen_radius", settingsId, 1.0, 0.0, 2.0);
  Parameter boost = Parameter("sharpen_boost", settingsId, 0.0,  0.0, 10.0);
  Oscillator amountOscillator = Oscillator(&amount);
  Oscillator radiusOscillator = Oscillator(&radius);
  Oscillator boostOscillator = Oscillator(&boost);
  SharpenSettings(std::string settingsId) : settingsId(settingsId) {
    parameters = {&amount, &radius, &boost};
    oscillators = {&amountOscillator, &radiusOscillator, &boostOscillator};
  }
};

struct PixelSettings: public JSONSerializable  {
  bool enabled = false;
  bool locked = true;
  std::string settingsId;
  
  Parameter scale;
  Parameter mix;
  Oscillator mixOscillator;
  Oscillator scaleOscillator;
  
  PixelSettings(std::string settingsId) :
  settingsId(settingsId),
  mixOscillator(&mix),
  scaleOscillator(&scale),
  scale(Parameter("pixel_scale", settingsId, 64.0,  0.0, 128.0)),
  mix(Parameter("pixel_mix", settingsId, 1.0, -2.0, 2.0))
  {
    parameters = {&scale, &mix};
    oscillators = {&scaleOscillator, &mixOscillator};
  }
};

// Feedback

enum FeedbackType {
  FeedbackType_Classic, FeedbackType_Luma, FeedbackType_Diff
};

struct FeedbackMixSettings: public JSONSerializable  {
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
  
  
  FeedbackMixSettings(std::string settingsId, int idx) :
  settingsId(settingsId),
  feedbackType(Parameter("feedback_type", settingsId, formatString("fb%dtype", idx), 0.0, 0.0, 2.0)),
  blend(Parameter("feedback_blend", settingsId, formatString("fb%dblend", idx), 0.0, 0.0, 1.0)),
  blendOscillator(Oscillator(&blend)),
  mix(Parameter("feedback_mix", settingsId, formatString("fb%dmix", idx), 0.0, 0.0, 1.0)),
  mixOscillator(Oscillator(&mix)),
  keyValue(Parameter("feedback_keyValue", settingsId, formatString("fb%dkey", idx), 0.0, 0.0, 1.0)),
  keyValueOscillator(Oscillator(&keyValue)),
  keyThreshold(Parameter("feedback_keyThreshold", settingsId, formatString("fb%dthresh", idx), 0.0, 0.0, 1.0)),
  keyThresholdOscillator(Oscillator(&keyThreshold)),
  delayAmount(Parameter("feedback_delayAmount", settingsId, 10.0, 0.0, 28.0)),
  delayAmountOscillator(Oscillator(&delayAmount))
  {
    parameters = {&blend, &mix, &keyValue, &keyThreshold, &delayAmount};
    oscillators = {&blendOscillator, &mixOscillator, &keyValueOscillator, &keyThresholdOscillator, &delayAmountOscillator};
  }
};

struct FeedbackMiscSettings: public JSONSerializable  {
  bool horizontalMirror;
  bool verticalMirror;
  Parameter rotate;
  Parameter xOffset;
  Parameter yOffset;
  Parameter scale;
  Oscillator rotationOscillator;
  Oscillator xOffsetOscillator;
  Oscillator yOffsetOscillator;
  Oscillator scaleOscillator;
  
  FeedbackMiscSettings(std::string settingsId, int idx) :
  rotate(Parameter("feedback_rotate", settingsId, 0.0001, 0.0001, TWO_PI)),
  xOffset(Parameter("feedback_xOffset", settingsId, 0.0, -30.0, 30.0)),
  yOffset(Parameter("feedback_yOffset", settingsId, 0.0, -30.0, 30.0)),
  scale(Parameter("feedback_scale", settingsId, 1.0, 0.001, 3.0)),
  xOffsetOscillator(Oscillator(&xOffset)),
  scaleOscillator(Oscillator(&scale)),
  yOffsetOscillator(Oscillator(&yOffset)),
  rotationOscillator(Oscillator(&rotate))
  {
    parameters = {&rotate, &xOffset, &yOffset, &scale};
    oscillators = {&rotationOscillator, &xOffsetOscillator, &yOffsetOscillator, &scaleOscillator};
  }
};

struct FeedbackSettings  {
  
  int index;
  Parameter enabled;
  std::string feedbackId;
  FeedbackMixSettings mixSettings;
  FeedbackMiscSettings miscSettings;
  HSBSettings hsbSettings;
  
  FeedbackSettings(std::string settingsId, int index) :
  index(index),
  enabled(Parameter(formatString("%sfeedback_enabled_%d", settingsId.c_str(), index), settingsId, formatString("fb%denabled", index), 0.0, 0.0, 0.0)),
  feedbackId(formatString("%s_%d_", settingsId.c_str(), index)),
  mixSettings(FeedbackMixSettings(feedbackId, index)),
  miscSettings(FeedbackMiscSettings(feedbackId, index)),
  hsbSettings(HSBSettings(feedbackId))
  {}
  
  json serialize(json j) {
    j = hsbSettings.serialize(j);
    j = mixSettings.serialize(j);
    return j;
  }
};

enum VideoSource { VideoSource_webcam, VideoSource_file };


struct VideoSettings   {
  HSBSettings hsbSettings;
  PixelSettings pixelSettings;
  BlurSettings blurSettings;
  SharpenSettings sharpenSettings;
  TransformSettings transformSettings;
  FeedbackSettings feedback0Settings;
  FeedbackSettings feedback1Settings;
  FeedbackSettings feedback2Settings;
  std::vector<FeedbackSettings*> allFeedbacks;
  int streamId;
  std::string settingsIdStr;
  
  VideoSettings(int intId, std::string strId) :
  streamId(intId),
  settingsIdStr(strId),
  hsbSettings(HSBSettings(strId)),
  pixelSettings(PixelSettings(strId)),
  blurSettings(BlurSettings(strId)),
  sharpenSettings(SharpenSettings(strId)),
  transformSettings(TransformSettings(strId)),
  feedback0Settings(FeedbackSettings(strId, 0)),
  feedback1Settings(FeedbackSettings(strId, 1)),
  feedback2Settings(FeedbackSettings(strId, 2)),
  
  allFeedbacks({
    &feedback0Settings,
    &feedback1Settings,
    &feedback2Settings})
  {
  }
  
  json serialize(json j) {
    j = hsbSettings.serialize(j);
    j = pixelSettings.serialize(j);
    j = blurSettings.serialize(j);
    j = sharpenSettings.serialize(j);
    j = transformSettings.serialize(j);
    j = feedback0Settings.serialize(j);
    j = feedback1Settings.serialize(j);
    j = feedback2Settings.serialize(j);
    return j;
  };
};



#endif /* Settings_h */
