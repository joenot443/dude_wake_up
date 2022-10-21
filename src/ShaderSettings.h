////
////  ShaderSettings.h
////  dude_wake_up
////
////  Created by Joe Crozier on 10/19/22.
////
//
//#ifndef ShaderSettings_h
//#define ShaderSettings_h
//#include "Parameter.h"
//#include "Oscillator.hpp"
//#include "VideoSettings.h"
//#include <string>
//
//using json = nlohmann::json;
//
//class ShaderSettings {
//  
//};
//
//class ShaderSerializable {
//public:
//  std::vector<Parameter*> parameters;
//  std::vector<Oscillator*> oscillators;
//  json serialize(json j) {
//    for (auto p : parameters) {
//      j[p->paramId] = p->paramValue();
//    }
//    auto ops = Oscillator::parametersFromOscillators(oscillators);
//    for (auto p : ops) {
//      j[p->paramId] = p->paramValue();
//    }
//    return j;
//  }
//};
//
//struct HSBShaderSettings: public ShaderSettings, public ShaderSerializable {
//  std::string settingsId;
//  Parameter hue;
//  Parameter saturation;
//  Parameter brightness;
//  bool invertHue = false;
//  bool invertSaturation = false;
//  bool invertBrightness = false;
//  Oscillator hueOscillator = Oscillator(&hue);
//  Oscillator saturationOscillator = Oscillator(&saturation);
//  Oscillator brightnessOscillator = Oscillator(&brightness);
//  
//  HSBShaderSettings(std::string settingsId) :
//  settingsId(settingsId),
//  hue(Parameter("HSB_hue", settingsId, 1.0, -1.0, 2.0)),
//  saturation(Parameter("HSB_saturation", settingsId, 1.0, -1.0, 3.0)),
//  brightness(Parameter("HSB_brightness", settingsId, 1.0, -1.0, 3.0))
//  
//  {
//    parameters = {&hue, &saturation, &brightness};
//    oscillators = {&hueOscillator, &saturationOscillator, &brightnessOscillator};
//  }
//};
//
//struct FeedbackShaderSettings: public ShaderSettings, public ShaderSerializable {};
//
//struct BlurShaderSettings: public ShaderSettings, public ShaderSerializable {};
//
//struct PixelateShaderSettings: public ShaderSettings, public ShaderSerializable {};
//
//#endif /* ShaderSettings_h */
