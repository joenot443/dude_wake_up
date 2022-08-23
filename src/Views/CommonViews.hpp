//
//  CommonViews.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 2022-05-12.
//

#ifndef CommonViews_hpp
#define CommonViews_hpp

#include <stdio.h>
#include "Oscillator.hpp"
#include "Parameter.h"

struct CommonViews {
  static void sSpacing();
  static void mSpacing();
  static void lSpacing();
  static void xlSpacing();
  static void Spacing(int n);
  
  static void MidiSelector(Parameter *videoParam);
  
  static void H3Title(std::string title);
  
  static void ModulationSelector(Parameter* videoParam);
  
  static void SliderWithOscillator(std::string title, std::string id, Parameter *param, Oscillator *o);
  
  static void IntSliderWithOscillator(std::string title, std::string id, Parameter *param, Oscillator *o);
  
  static void SliderWithInvertOscillator(std::string title, std::string id, Parameter *param, bool *invert, Oscillator *o);
};

#endif /* CommonViews_hpp */
