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
  std::string paramId;
  float value = 0.0;
  // Another Parameter which is driving this one's value
  Parameter* driver = NULL;
  
  float min = 0.0;
  float max = 1.0;
  
  int intValue() {
    return static_cast<int>(value);
  }
  
  float percentValue() {
    return value/(max - min);
  }
  
  /// Drives the Parameter between its min and max by a float percent.
  void driveValue(float percent) {
    float range = max - min;
    value = percent * range + min;
  }
  Parameter(std::string name, std::string settingsId, float value);
  Parameter(std::string name, std::string settingsId, float value, float min, float max);
};

#endif /* BaseField_h */
