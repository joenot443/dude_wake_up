//
//  CommonViews.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 2022-05-12.
//

#ifndef CommonViews_hpp
#define CommonViews_hpp

#include <stdio.h>
#include "imgui_node_editor.h"
#include "Oscillator.hpp"
#include "Parameter.hpp"
#include "VideoSource.hpp"
#include "ofMain.h"
#include "ofxImGui.h"

namespace ed = ax::NodeEditor;

class ShaderSource;

class Node;

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
  
  static void PaddedText(std::string text, ImVec2 padding);
  static bool InvisibleIconButton(std::string id);
  
  static bool IconButton(const char *icon, std::string id);
  
  static bool LargeIconButton(const char *icon, std::string id, bool enabled = true);
  
  static bool XLargeIconButton(const char *icon, std::string id);
  
  static void IconTitle(const char *icon);
  
  static void XLargeIconTitle(const char *icon);
  
  static void ImageNamed(std::string filename, double width, double height);
  
  static void ColorShaderStageParameter(std::shared_ptr<Parameter> param);
  
  static void CheckboxShaderStageParameter(std::shared_ptr<Parameter> param);
  
  static void IntShaderStageParameter(std::shared_ptr<Parameter> param);
  
  static void ShaderStageParameter(std::shared_ptr<Parameter> param, std::shared_ptr<Oscillator> osc);
  
  static bool ShaderParameter(std::shared_ptr<Parameter> param, std::shared_ptr<Oscillator> osc);
  
  static bool MiniSlider(std::shared_ptr<Parameter> param);
  
  static bool MiniVSlider(std::shared_ptr<Parameter> param);
  
  static bool Selector(std::shared_ptr<Parameter> param, std::vector<std::string> options);
  
  static bool ShaderCheckbox(std::shared_ptr<Parameter> param, bool sameLine = false);
  
  static void ShaderColor(std::shared_ptr<Parameter> param);
  
  static void ShaderIntParameter(std::shared_ptr<Parameter> param);
  
  static bool ShaderOption(std::shared_ptr<Parameter> param, std::vector<std::string> options, bool drawTitle = true);
  
  static bool TextureFieldAndBrowser(std::shared_ptr<Parameter> param);
  
  static bool IconFieldAndBrowser(std::shared_ptr<Parameter> param);
  
  static void HorizontallyAligned(float width, float alignment = 0.5);
  
  static void MidiSelector(std::shared_ptr<Parameter> videoParam);
  
  static void H1Title(std::string title, bool padding = true);
  
  static void H2Title(std::string title, bool padding = true);
  
  static void H3Title(std::string title, bool padding = true);
  
	static void H3BTitle(std::string title, bool padding = true);
  
  static void H4Title(std::string title, bool padding = true);
  
  static void CenteredVerticalLine();
  
  static void ModulationSelector(std::shared_ptr<Parameter> videoParam);
  
  static void FavoriteButton(std::shared_ptr<Parameter> param);
  
  static void AudioParameterSelector(std::shared_ptr<Parameter> videoParam);
  
  static bool Slider(std::string title, std::string id, std::shared_ptr<Parameter> param, float width = 200.0f);
  
  static void MultiSlider(std::string title, std::string id, std::shared_ptr<Parameter> param1, std::shared_ptr<Parameter> param2,
                          std::shared_ptr<Oscillator> param1Oscillator,
                          std::shared_ptr<Oscillator> param2Oscillator);
  
  static bool AreaSlider(std::string id,
                         std::shared_ptr<Parameter> minX, std::shared_ptr<Parameter> maxX,
                         std::shared_ptr<Parameter> minY, std::shared_ptr<Parameter> maxY,
                         std::shared_ptr<Oscillator> minXOscillator, std::shared_ptr<Oscillator> maxXOscillator,
                         std::shared_ptr<Oscillator> minYOscillator, std::shared_ptr<Oscillator> maxYOscillator) ;
  
  static void RangeSlider(std::string title, std::string id, std::shared_ptr<Parameter> param, std::shared_ptr<Parameter> param2, std::string label, bool dirty = false);
  
  static void PlaybackSlider(std::shared_ptr<Parameter> param, float length);
  
  static void PlaybackRangeSlider(std::string title, std::string id, std::shared_ptr<Parameter> param, std::shared_ptr<Parameter> param2, float duration, bool dirty = false);
  
  static bool SelectionRect(ImVec2* start_pos, ImVec2* end_pos, ImVec2 topLeft, ImVec2 bottomRight);
  
  static void BlendModeSelector(std::shared_ptr<Parameter> param, std::shared_ptr<Parameter> flip = nullptr, std::shared_ptr<Parameter> alpha = nullptr, std::shared_ptr<Oscillator> alphaOscillator = nullptr, std::shared_ptr<Parameter> blendWithEmpty = nullptr);
  
  static bool ResetButton(std::string id, std::shared_ptr<Parameter> param);
  static void OscillateButton(std::string id, std::shared_ptr<Oscillator> o, std::shared_ptr<Parameter> param);
  static void OscillatorWindow(std::shared_ptr<Oscillator> o, std::shared_ptr<Parameter> param);
  
  static bool IsMouseWithin(ImVec2 topLeft, ImVec2 bottomRight, ImVec2 offset = ImVec2(0.0, 0.0));
  
  // Redesign
  
  static void PushRedesignStyle();
  static void PopRedesignStyle();
  
  static bool SelectorTitleButton(std::string title, float width);
  
  static void DownChevron(ImVec2 pos, float scale);
  
  static void NodePin(std::shared_ptr<Node> node);
  static ed::PinId InputNodePin(std::shared_ptr<Node> node, InputSlot slot);
  static ed::PinId OutputNodePin(std::shared_ptr<Node> node, OutputSlot slot);
  
  static void ImageNamedNew(std::string name, float width, float height);
  
  static bool ImageButton(std::shared_ptr<Node> node, std::string imageName);
  
};

static ImVec2 getScaledWindowSize() {
  int scale = dynamic_cast<ofAppGLFWWindow*>(ofGetWindowPtr())->getPixelScreenCoordScale();
  return ImVec2(ofGetWindowWidth() / scale, ofGetWindowHeight() / scale);
}

static ImVec2 getScaledScreenSize() {
  int scale = dynamic_cast<ofAppGLFWWindow*>(ofGetWindowPtr())->getPixelScreenCoordScale();
  return ImVec2(ofGetScreenWidth() / scale, ofGetScreenHeight() / scale);
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
  return ImVec2(pos.x / scale, pos.y / scale);
}

static ImVec2 scaleVector(ImVec2 vec) {
  int scale = dynamic_cast<ofAppGLFWWindow*>(ofGetWindowPtr())->getPixelScreenCoordScale();
  return scale * vec;
}

static float scaleFloat(float f) {
  int scale = dynamic_cast<ofAppGLFWWindow*>(ofGetWindowPtr())->getPixelScreenCoordScale();
  return scale * f;
}

#endif /* CommonViews_hpp */
