//
//  BaseField.h
//  dude_wake_up
//
//  Created by Joe Crozier on 2022-07-04.
//

#include <stdio.h>
#include <string>

#ifndef BaseField_h
#define BaseField_h

struct Parameter
{
  std::string name = "";
  std::string shaderKey = "";
  std::string paramId;
  float defaultValue = 0.0;
  float value = 0.0;
  int intValue = 0;
  bool boolValue = false;
  // Another Parameter which is driving this one's value
  Parameter* driver = NULL;
  
  float min = 0.0;
  float max = 1.0;
  
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
  float valueRespectingBool() {
    if (boolValue) {
      return true;
    }
    return value;
  }
  
  Parameter(std::string name, std::string settingsId, float value);
  Parameter(std::string name, std::string settingsId, std::string shaderKey, float value);
  Parameter(std::string name, std::string settingsId, std::string shaderKey, float value, float min, float max);
  Parameter(std::string name, std::string settingsId, float value, float min, float max);
};

#endif /* BaseField_h */
