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
#include "Strings.h"
#include <string>

const int FrameBufferCount=30;

// Basic

struct HSBSettings {
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
  hue(Parameter("HSB_hue", settingsId, 1.0, -5.0, 6.0)),
  saturation(Parameter("HSB_saturation", settingsId, 1.0, -5.0, 6.0)),
  brightness(Parameter("HSB_brightness", settingsId, 1.0, -5.0, 6.0))
  {}
};

struct BlurSettings {
  std::string settingsId;
  Parameter amount;
  Parameter radius;
  Oscillator amountOscillator = Oscillator(&amount);
  Oscillator radiusOscillator = Oscillator(&radius);
  BlurSettings(std::string settingsId) :
  settingsId(settingsId),
  amount(Parameter("blur_amount", settingsId, 0.0, -2.0, 2.0)),
  radius(Parameter("blur_radius", settingsId, 1.0, -2.0, 2.0))
  {}
};

struct SharpenSettings {
  std::string settingsId;
  Parameter amount = Parameter("sharpen_amount", settingsId, 0.0, -2.0, 2.0);
  Parameter radius = Parameter("sharpen_radius", settingsId, 1.0, -2.0, 2.0);
  Parameter boost = Parameter("sharpen_boost", settingsId, 0.0,  0.0, 10.0);
  Oscillator amountOscillator = Oscillator(&amount);
  Oscillator radiusOscillator = Oscillator(&radius);
  Oscillator boostOscillator = Oscillator(&boost);
  SharpenSettings(std::string settingsId) : settingsId(settingsId) {
    
  }
};

struct PixelSettings {
  bool enabled = false;
  bool locked = true;
  std::string settingsId;
  
  Parameter scaleX;
  Parameter oscScaleX;
  Parameter scaleY;
  Parameter oscScaleY;
  Parameter mix;
  Oscillator mixOscillator;
  Oscillator scaleXOscillator;
  Oscillator scaleYOscillator;
  
  PixelSettings(std::string settingsId) :
  settingsId(settingsId),
  mixOscillator(&mix),
  scaleXOscillator(&scaleX),
  scaleYOscillator(&scaleY),
  scaleX(Parameter("pixel_scaleX", settingsId, 64.0,  0.0, 256.0)),
  oscScaleX(Parameter("pixel_oscScaleX", settingsId, 64.0,  0.0, 256.0)),
  scaleY(Parameter("pixel_scaleY", settingsId, 64.0, 0.0, 256.0)),
  oscScaleY(Parameter("pixel_oscScaleX", settingsId, 64.0, 0.0, 256.0)),
  mix(Parameter("pixel_mix", settingsId, 1.0, -2.0, 2.0))
  {}
  
  void tick() {
    if (!scaleXOscillator.enabled && !scaleYOscillator.enabled) {
      scaleY.value = scaleX.value;
      return;
    }
    
    // Only lock the X/Y values together if either Oscillator is running
    if (scaleXOscillator.enabled) {
      scaleX.value = floor(oscScaleX.value);
    }
    if (scaleYOscillator.enabled || locked) {
      scaleY.value = locked ? floor(oscScaleX.value) : floor(oscScaleY.value);
    }
  }
};

struct BasicSettings {
  HSBSettings hsbSettings;
  PixelSettings pixelSettings;
  BlurSettings blurSettings;
  SharpenSettings sharpenSettings;
  
  BasicSettings(std::string settingsId) :
  hsbSettings(HSBSettings(settingsId)),
  pixelSettings(PixelSettings(settingsId)),
  blurSettings(BlurSettings(settingsId)),
  sharpenSettings(SharpenSettings(settingsId))
  {}
};

// Feedback

struct FeedbackMixSettings {
  std::string settingsId;
  
  
  Parameter mix;
  Oscillator mixOscillator;
  Parameter keyValue;
  Oscillator keyValueOscillator;
  Parameter keyThreshold;
  Oscillator keyThresholdOscillator;
  Parameter delayAmount;
  Oscillator delayAmountOscillator;
  
  FeedbackMixSettings(std::string settingsId) :
  mix(Parameter("feedback_mix", settingsId, 0.0, 0.0, 1.0)),
  keyValue(Parameter("feedback_keyValue", settingsId, 0.0, 0.0, 1.0)),
  keyThreshold(Parameter("feedback_keyThreshold", settingsId, 0.0, 0.0, 1.0)),
  delayAmount(Parameter("feedback_delayAmount", settingsId, 10.0, 0.0, 30.0)),
  mixOscillator(Oscillator(&mix)),
  keyValueOscillator(Oscillator(&keyValue)),
  keyThresholdOscillator(Oscillator(&keyThreshold)),
  delayAmountOscillator(Oscillator(&delayAmount))
  {}
};

struct FeedbackMiscSettings {
  bool horizontalMirror;
  bool verticalMirror;
  Parameter rotate;
  Parameter xOffset;
  Parameter yOffset;
  Oscillator rotationOscillator;
  Oscillator xOffsetOscillator;
  Oscillator yOffsetOscillator;
  
  FeedbackMiscSettings(std::string settingsId) :
  rotate(Parameter("feedback_rotate", settingsId, 0.0001, 0.0001, TWO_PI)),
  xOffset(Parameter("feedback_xOffset", settingsId, 0.0, -300.0, 300.0)),
  yOffset(Parameter("feedback_yOffset", settingsId, 0.0, -300.0, 300.0)),
  xOffsetOscillator(Oscillator(&xOffset)),
  yOffsetOscillator(Oscillator(&yOffset)),
  rotationOscillator(Oscillator(&rotate))
  {}
};

struct FeedbackScaleSettings {
  Parameter xScale;
  Parameter yScale;
  FeedbackScaleSettings(std::string settingsId) :
  xScale(Parameter("feedback_scaleX", settingsId, 0.0, 5.0, 1.0)),
  yScale(Parameter("feedback_scaleY", settingsId, 0.0, 5.0, 1.0)) {}
};

struct FeedbackSettings {
  int index;
  bool enabled = false;
  std::string feedbackId;
  FeedbackMixSettings mixSettings;
  FeedbackMiscSettings miscSettings;
  HSBSettings hsbSettings;
  FeedbackScaleSettings scaleSettings;
  
  FeedbackSettings(std::string settingsId, int index) :
  feedbackId(formatString("%s_%d_", settingsId.c_str(), index)),
  mixSettings(FeedbackMixSettings(feedbackId)),
  miscSettings(FeedbackMiscSettings(feedbackId)),
  hsbSettings(HSBSettings(feedbackId)),
  scaleSettings(FeedbackScaleSettings(feedbackId)),
  index(index)
  {}
};

// Misc

struct CameraSettings {
  bool horizontalFlip;
  bool verticalFlip;
  float scale;
  float xSkew;
  float ySkew;
};

struct MiscRenderSettings {
  bool renderTetrahedron;
  bool renderHyperCube;
};

struct MiscSettings {
  CameraSettings cameraSettgs;
  MiscRenderSettings miscRenderSettings;
};

enum VideoSource { VideoSource_webcam, VideoSource_file };

struct VideoSettings {
  BasicSettings basicSettings;
  MiscSettings miscSettings;
  FeedbackSettings feedback1Settings;
  FeedbackSettings feedback2Settings;
  FeedbackSettings feedback3Settings;
  
  int streamId;
  
  VideoSettings(std::string settingsId) :
  basicSettings(BasicSettings(settingsId)),
  miscSettings(MiscSettings()),
  feedback1Settings(FeedbackSettings(settingsId, 0)),
  feedback2Settings(FeedbackSettings(settingsId, 1)),
  feedback3Settings(FeedbackSettings(settingsId, 2))
  {
  }
};



#endif /* Settings_h */
