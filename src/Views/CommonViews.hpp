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
#include "VideoSource.hpp"
#include "ofxImGui.h"

struct CommonViews
{
  static ImVec2 windowCanvasSize();

  static void xsSpacing();
  static void sSpacing();
  static void mSpacing();
  static void lSpacing();
  static void xlSpacing();
  static void blankSpacing(int n);
  static void Spacing(int n);
  static void HSpacing(int n);

  static bool IconButton(const char *icon, std::string id);

  static bool LargeIconButton(const char *icon, std::string id);

  static void IconTitle(const char *icon);  

  static void ShaderParameter(std::shared_ptr<Parameter> param, std::shared_ptr<Oscillator> osc);

  static void ShaderCheckbox(std::shared_ptr<Parameter> param);
  
  static void ShaderOption(std::shared_ptr<Parameter> param, std::vector<std::string> options);

  static void HorizontallyAligned(float width, float alignment = 0.5);

  static void MidiSelector(std::shared_ptr<Parameter> videoParam);

  static void H3Title(std::string title);

  static void H4Title(std::string title);

  static void ResolutionSelector(std::shared_ptr<VideoSource> source);

  static void CenteredVerticalLine();

  static void ModulationSelector(std::shared_ptr<Parameter> videoParam);

  static void AudioParameterSelector(std::shared_ptr<Parameter> videoParam);

  static bool Slider(std::string title, std::string id, std::shared_ptr<Parameter> param);
  
  static void RangeSlider(std::string title, std::string id, std::shared_ptr<Parameter> param, std::shared_ptr<Parameter> param2, float duration, bool dirty = false);

  static void PlaybackSlider(std::shared_ptr<Parameter> param, float length);

  static void ShaderColor(std::shared_ptr<Parameter> param);

  static void IntSlider(std::string title, std::string id, std::shared_ptr<Parameter> param);

  static void ResetButton(std::string id, std::shared_ptr<Parameter> param);
  static void OscillateButton(std::string id, std::shared_ptr<Oscillator> o, std::shared_ptr<Parameter> param);
  static void OscillatorWindow(std::shared_ptr<Oscillator> o, std::shared_ptr<Parameter> param);
};

#endif /* CommonViews_hpp */
