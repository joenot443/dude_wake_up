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
#include "ofMain.h"
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

  static bool InvisibleIconButton(std::string id);
  
  static bool IconButton(const char *icon, std::string id);

  static bool LargeIconButton(const char *icon, std::string id);

  static void IconTitle(const char *icon);
  
  static void ShaderStageParameter(std::shared_ptr<FavoriteParameter> param, std::shared_ptr<Oscillator> osc);

  static void ShaderParameter(std::shared_ptr<Parameter> param, std::shared_ptr<Oscillator> osc);

  static bool ShaderCheckbox(std::shared_ptr<Parameter> param, bool sameLine = false);
  
  static void ShaderColor(std::shared_ptr<Parameter> param);

  static void ShaderIntParameter(std::shared_ptr<Parameter> param);
  
  static bool ShaderOption(std::shared_ptr<Parameter> param, std::vector<std::string> options);
  
  static bool TextureFieldAndBrowser(std::shared_ptr<Parameter> param);
  
  static bool IconFieldAndBrowser(std::shared_ptr<Parameter> param);

  static void HorizontallyAligned(float width, float alignment = 0.5);

  static void MidiSelector(std::shared_ptr<Parameter> videoParam);
	
  static void H1Title(std::string title);
  
  static void H2Title(std::string title);
  
  static void H3Title(std::string title);

  static void H4Title(std::string title);

  static void ResolutionSelector(std::shared_ptr<VideoSource> source);

  static void CenteredVerticalLine();

  static void ModulationSelector(std::shared_ptr<Parameter> videoParam);
  
  static void FavoriteButton(std::shared_ptr<Parameter> param);

  static void AudioParameterSelector(std::shared_ptr<Parameter> videoParam);

  static bool Slider(std::string title, std::string id, std::shared_ptr<Parameter> param);
  
  static bool MultiSlider(std::string title, std::string id, std::shared_ptr<Parameter> param1, std::shared_ptr<Parameter> param2,
                                  std::shared_ptr<Oscillator> param1Oscillator,
                                  std::shared_ptr<Oscillator> param2Oscillator);
  
  static void RangeSlider(std::string title, std::string id, std::shared_ptr<Parameter> param, std::shared_ptr<Parameter> param2, float duration, bool dirty = false);

  static void PlaybackSlider(std::shared_ptr<Parameter> param, float length);

  static void ResetButton(std::string id, std::shared_ptr<Parameter> param);
  static void OscillateButton(std::string id, std::shared_ptr<Oscillator> o, std::shared_ptr<Parameter> param);
  static void OscillatorWindow(std::shared_ptr<Oscillator> o, std::shared_ptr<Parameter> param);
};

static ImVec2 getScaledWindowSize() {
  int scale = dynamic_cast<ofAppGLFWWindow*>(ofGetWindowPtr())->getPixelScreenCoordScale();
  return ImVec2(ofGetWindowWidth() / scale, ofGetWindowHeight() / scale);
}

static float getScaledWindowWidth() {
  int scale = dynamic_cast<ofAppGLFWWindow*>(ofGetWindowPtr())->getPixelScreenCoordScale();
  return ofGetWindowWidth() / scale;
}

static float getScaledWindowHeight() {
  int scale = dynamic_cast<ofAppGLFWWindow*>(ofGetWindowPtr())->getPixelScreenCoordScale();
  return ofGetWindowHeight() / scale;
}

static ImVec2 getScaledMouseLocation() {
  int scale = dynamic_cast<ofAppGLFWWindow*>(ofGetWindowPtr())->getPixelScreenCoordScale();
  return ImVec2(ofGetMouseX() / scale, ofGetMouseY() / scale);
}

static ImVec2 getScaledCursorScreenLocation() {
  int scale = dynamic_cast<ofAppGLFWWindow*>(ofGetWindowPtr())->getPixelScreenCoordScale();
  auto pos = ImGui::GetCursorScreenPos();
  return ImVec2(pos.x * scale, pos.y * scale);
}

static ImVec2 getScaledContentRegionAvail() {
  int scale = dynamic_cast<ofAppGLFWWindow*>(ofGetWindowPtr())->getPixelScreenCoordScale();
  auto pos = ImGui::GetContentRegionAvail();
  return ImVec2(pos.x * scale, pos.y * scale);
}

#endif /* CommonViews_hpp */
