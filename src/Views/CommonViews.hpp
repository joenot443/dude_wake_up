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
#include "Parameter.hpp"
#include "ofxImGui.h"

struct CommonViews {
  static ImVec2 windowCanvasSize();
  
  static void sSpacing();
  static void mSpacing();
  static void lSpacing();
  static void xlSpacing();
  static void Spacing(int n);
  static void HSpacing(int n);
  
  static bool IconButton(const char* icon, std::string id);
  
  static void IconTitle(const char* icon);
  
  static void ShaderParameter(std::shared_ptr<Parameter> param, std::shared_ptr<Oscillator> osc);

  static void ShaderCheckbox(std::shared_ptr<Parameter> param);

  static void HorizontallyAligned(float width, float alignment = 0.5);
  
  static void MidiSelector(std::shared_ptr<Parameter> videoParam);
  
  static void H3Title(std::string title);
  
  static void H4Title(std::string title);
  
  static void CenteredVerticalLine();
  
  static void ModulationSelector(std::shared_ptr<Parameter> videoParam);
  
  static void AudioParameterSelector(std::shared_ptr<Parameter> videoParam);
  
  static void Slider(std::string title, std::string id, std::shared_ptr<Parameter> param);
  static void IntSlider(std::string title, std::string id, std::shared_ptr<Parameter> param);
    
  static void ResetButton(std::string id, std::shared_ptr<Parameter> param);
  static void OscillateButton(std::string id, std::shared_ptr<Oscillator> o, std::shared_ptr<Parameter> param);
  static void OscillatorWindow(std::shared_ptr<Oscillator> o, std::shared_ptr<Parameter> param);
};

#endif /* CommonViews_hpp */
