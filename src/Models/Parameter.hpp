//
//  BaseField.h
//  dude_wake_up
//
//  Created by Joe Crozier on 2022-07-04.
//

#include "json.hpp"
#include "Strings.hpp"
#include <stdio.h>
#include <string>
#include <memory>
#include "ofMain.h"

#ifndef BaseField_h
#define BaseField_h

using json = nlohmann::json;

typedef enum {
  ParameterType_Standard,
  ParameterType_Int,
  ParameterType_Bool,
  ParameterType_Color,
  ParameterType_Hidden
} ParameterType;

struct Parameter : public std::enable_shared_from_this<Parameter>
{
  std::string name = "";
  std::string ownerName = "";
  std::string ownerSettingsId = "";
  std::string paramId;
  std::string midiDescriptor = "";
  float defaultValue = 0.0;
  float value = 0.0;
  int intValue = 0;
  bool boolValue = false;
  bool favorited = false;
  
  // Whether the Parameter is within a connected Node
  bool active = false;
  
  // Whether the Parameter should be hidden from Node view
  bool hiddenFromNode = false;
  
  // The options associated with this Parameter when used in Int mode
  std::vector<std::string> options = {};
  
  // Another Parameter which is driving this one's value
  std::shared_ptr<Parameter> driver = nullptr;
  std::shared_ptr<Parameter> shift = nullptr;
  std::shared_ptr<Parameter> scale = nullptr;
  
  ParameterType type;

  // Color value contained by Parameter
  std::shared_ptr<std::array<float, 4>> color = std::make_shared<std::array<float, 4>>(std::array<float, 4>({0.0f, 0.0f, 0.0f, 1.0f}));

  float min = 0.0;
  float max = 1.0;
  
  void setColor(std::array<float, 4> newColor) {
    color = std::make_shared<std::array<float, 4>>(newColor);
  }

  json serialize()
  {
    json j;
    j["name"] = name;
    j["paramId"] = paramId;
    j["midiDescriptor"] = midiDescriptor;
    j["value"] = value;
    j["intValue"] = intValue;
    j["boolValue"] = boolValue;
    j["favorited"] = favorited;
    
    // If the color RGB values are not all 0, add them to the json
    if (color->at(0) != 0.0 || color->at(1) != 0.0 || color->at(2) != 0.0 || color->at(3) != 0.0)
    {
      j["r"] = color->at(0);
      j["g"] = color->at(1);
      j["b"] = color->at(2);
      j["a"] = color->at(3);
    }
    return j;
  }

  void addDriver(std::shared_ptr<Parameter> dr)
  {
    driver = dr;
    shift = std::make_shared<Parameter>("shift", 0.5, -1.0, 1.0);
    scale = std::make_shared<Parameter>("scale", 0.2, 0.0, 2.0);
  }
  
  void removeDriver() {
    driver = nullptr;
    shift = nullptr;
    scale = nullptr;
  }

  void load(json j)
  {
    if (j.is_number())
    {
      setValue(j);
    }

    if (j.is_object())
    {
      if (j.contains("value"))
      {
        value = j["value"];
        setValue(j["value"]);
      }
      if (j.contains("intValue"))
      {
        intValue = j["intValue"];
      }
      if (j.contains("boolValue"))
      {
        boolValue = j["boolValue"];
      }
      if (j.contains("midiDescriptor"))
      {
        midiDescriptor = j["midiDescriptor"];
      }
      if (j.contains("r"))
      {
        color->at(0) = j["r"];
        color->at(1) = j["g"];
        color->at(2) = j["b"];
        color->at(3) = j["a"];
      }
      if (j.contains("favorited")) {
        favorited = j["favorited"];
      }
    }
  }

  void tick();

  std::string description() { return std::to_string(value); }

  float percentValue() { return value / (max - min); }

  /// Resets the value to its defaultValue
  void resetValue() { setValue(defaultValue); }

  /// Sets the int, bool, and float values of the Parameter to the passed
  /// argument
  void setValue(float newValue)
  {
    value = newValue;
    intValue = static_cast<int>(newValue);
    boolValue = newValue > 0.0001;
  }
  
  void setBoolValue(bool newValue) {
    value = newValue ? 1.0 : 0.0;
    intValue = newValue ? 1 : 0;
    boolValue = newValue;
  }
  
  /// Sets the int and bool values of the Parameter to the float value.
  void affirmValue()
  {
    intValue = static_cast<int>(value);
    boolValue = value > 0.0001;
  }
  
  
  /// Sets the float and bool values of the Parameter to the int value.
  void affirmIntValue()
  {
    value = static_cast<float>(intValue);
    boolValue = value > 0.0001;
  }
  
  /// Toggles the bool value of the Parameter and updates the float and int values accordingle
  void toggleValue()
  {
    boolValue = !boolValue;
    value = boolValue ? 1.0 : 0.0;
    intValue = boolValue ? 1 : 0;
  }
  
  /// Drives the Parameter between its min and max by a float percent.
  void driveValue(float percent)
  {
    float range = max - min;
    setValue(percent * range + min);
  }
  
  void scaleAudioValue(float percent)
  {
    float range = max - min;
    setValue(fmax(min, fmin(percent * range * scale->value + shift->value * range, max)));
  }

  /// Returns 1 if our boolValue is true, otherwise the normal float value
  float paramValue()
  {
    if (boolValue)
    {
      return true;
    }

    if (intValue != 0)
    {
      return static_cast<float>(intValue);
    }

    return value;
  }

  int intParamValue()
  {
    if (boolValue)
    {
      return intValue;
    }

    if (intValue != 0)
    {
      return intValue;
    }

    return static_cast<int>(value);
  }

  std::string oscPopupId()
  {
    return formatString("%s Oscillator##osc_popup", name.c_str());
  }

  std::string audioPopupId()
  {
    return formatString("##%s_audio_popup", name.c_str());
  }
	
  Parameter(std::string name, ParameterType type);
  Parameter(std::string name, float value = 0.0, ParameterType = ParameterType_Standard);
  Parameter(std::string name, float value, float min, float max, ParameterType = ParameterType_Standard);

private:
  Parameter(){};
};

#endif /* BaseField_h */
