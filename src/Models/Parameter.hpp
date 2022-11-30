//
//  BaseField.h
//  dude_wake_up
//
//  Created by Joe Crozier on 2022-07-04.
//

#include <stdio.h>
#include <string>
#include "json.hpp"

#ifndef BaseField_h
#define BaseField_h

using json = nlohmann::json;

struct Parameter
{
  std::string name = "";
  std::string shaderKey = "";
  std::string paramId;
  std::string midiDescriptor = "";
  float defaultValue = 0.0;
  float value = 0.0;
  int intValue = 0;
  bool boolValue = false;
  // Another Parameter which is driving this one's value
  std::shared_ptr<Parameter> driver = nullptr;
  
  float min = 0.0;
  float max = 1.0;

  json serialize() {
    json j;
    j["name"] = name;
    j["paramId"] = paramId;
    j["midiDescriptor"] = midiDescriptor;
    j["value"] = value;
    return j;
  }

  void load(json j) {
    if (j.is_object()) {
      if (j.contains("value")) {
        value = j["value"];
      }
      if (j.contains("intValue")) {
        intValue = j["intValue"];
      }
      if (j.contains("boolValue")) {
        boolValue = j["boolValue"];
      }
      if (j.contains("midiDescriptor")) {
        midiDescriptor = j["midiDescriptor"];
      }
    }
  }
  
  std::string description() {
    return std::to_string(value);
  }
  
  float percentValue() {
    return value/(max - min);
  }

  /// Resets the value to its defaultValue
  void resetValue() {
    setValue(defaultValue); 
  }
  
  /// Sets the int, bool, and float values of the Parameter to the passed argument
  void setValue(float newValue) {
    value = newValue;
    intValue = static_cast<int>(newValue);
    boolValue = newValue > 0.0001;
  }
  /// Drives the Parameter between its min and max by a float percent.
  void driveValue(float percent) {
    float range = max - min;
    setValue(percent * range + min);
  }
  
  /// Returns 1 if our boolValue is true, otherwise the normal float value
  float paramValue() {
    if (boolValue) {
      return true;
    }
    
    if (intValue != 0) {
      return static_cast<float>(intValue);
    }
    
    return value;
  }
  
  int intParamValue() {
    if (boolValue) {
      return 1;
    }
    
    if (intValue != 0) {
      return intValue;
    }
    
    return static_cast<int>(value);    
  }
  
  Parameter(std::string name, std::string settingsId, float value);
  Parameter(std::string name, std::string settingsId, std::string shaderKey, float value);
  Parameter(std::string name, std::string settingsId, std::string shaderKey, float value, float min, float max);
  Parameter(std::string name, std::string settingsId, float value, float min, float max);
};

#endif /* BaseField_h */
