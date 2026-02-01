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

  static bool ShaderMiniParameter(std::shared_ptr<Parameter> param, std::shared_ptr<Oscillator> osc);

  static bool MiniSlider(std::shared_ptr<Parameter> param, bool sameLine = true, ImGuiSliderFlags flags = ImGuiSliderFlags_None);
  
  static bool MiniVSlider(std::shared_ptr<Parameter> param);
  
  static bool Selector(std::shared_ptr<Parameter> param, std::vector<std::string> options, float width = 200.0f);
  
  static bool ShaderCheckbox(std::shared_ptr<Parameter> param, bool sameLine = false);
  
  static void ShaderColor(std::shared_ptr<Parameter> param);
  
  static bool ShaderIntParameter(std::shared_ptr<Parameter> param);
  
  static bool ShaderOption(std::shared_ptr<Parameter> param, std::vector<std::string> options, bool drawTitle = true);
  
  static bool TextureFieldAndBrowser(std::shared_ptr<Parameter> param);
  
  static bool IconFieldAndBrowser(std::shared_ptr<Parameter> param);
  
  static void HorizontallyAligned(float width, float alignment = 0.5);
  
  static void MidiSelector(std::shared_ptr<Parameter> videoParam, ImVec2 size = ImVec2(105.0, 50.0));
  
  static void H1Title(std::string title, bool padding = true);
  
  static void H2Title(std::string title, bool padding = true);
  
  static void H3Title(std::string title, bool padding = true);
  
	static void H3BTitle(std::string title, bool padding = true);
  
  static void H4Title(std::string title, bool padding = true);
  
  static void CenteredVerticalLine();
  
  static void ModulationSelector(std::shared_ptr<Parameter> videoParam);
  
  static void FavoriteButton(std::shared_ptr<Parameter> param, ImVec2 size = ImVec2(105.0, 50.0), ImVec2 imageRatio = ImVec2(3.0, 2.0));
  
  static void AudioParameterSelector(std::shared_ptr<Parameter> videoParam, ImVec2 size = ImVec2(105.0, 50.0));

  static void OscillateButton(std::string id, std::shared_ptr<Oscillator> o, std::shared_ptr<Parameter> param, ImVec2 size = ImVec2(105.0, 50.0), ImVec2 imageRatio = ImVec2(3.0, 2.0));
  
  static bool Slider(std::string title, std::string id, std::shared_ptr<Parameter> param, float width = 200.0f);
  
  static bool IntSlider(std::string title, std::string id, std::shared_ptr<Parameter> param, float width = 200.0f);
  
  static void MultiSlider(std::string title, std::string id, std::shared_ptr<Parameter> param1, std::shared_ptr<Parameter> param2,
                          std::shared_ptr<Oscillator> param1Oscillator,
                          std::shared_ptr<Oscillator> param2Oscillator,
                          float aspectRatio = 0.5625);
  
  static bool AreaSlider(std::string id,
                         std::shared_ptr<Parameter> minX, std::shared_ptr<Parameter> maxX,
                         std::shared_ptr<Parameter> minY, std::shared_ptr<Parameter> maxY,
                         std::shared_ptr<Oscillator> minXOscillator, std::shared_ptr<Oscillator> maxXOscillator,
                         std::shared_ptr<Oscillator> minYOscillator, std::shared_ptr<Oscillator> maxYOscillator) ;
  
  static void RangeSlider(std::string title, std::string id, std::shared_ptr<Parameter> param, std::shared_ptr<Parameter> param2, std::string label, bool dirty = false);
  
  static bool PlaybackSlider(std::shared_ptr<Parameter> param, float length, float width = 300.0);
  
  static void PlaybackRangeSlider(std::string title, std::string id, std::shared_ptr<Parameter> param, std::shared_ptr<Parameter> param2, float duration, bool dirty = false);
  
  static bool SelectionRect(ImVec2* start_pos, ImVec2* end_pos, ImVec2 topLeft, ImVec2 bottomRight);
  
  static void BlendModeSelector(std::shared_ptr<Parameter> param, std::shared_ptr<Parameter> flip = nullptr, std::shared_ptr<Parameter> alpha = nullptr, std::shared_ptr<Oscillator> alphaOscillator = nullptr, std::shared_ptr<Parameter> amount = nullptr, std::shared_ptr<Oscillator> amountOscillator = nullptr, std::shared_ptr<Parameter> blendWithEmpty = nullptr);
  
  static bool ResetButton(std::string id, std::shared_ptr<Parameter> param, ImVec2 size = ImVec2(25.0, 25.0), ImVec2 padding = ImVec2(7.5, 7.5));
  
  static void OscillatorWindow(std::shared_ptr<Oscillator> o, std::shared_ptr<Parameter> param);
  
  static bool IsMouseWithin(ImVec2 topLeft, ImVec2 bottomRight, ImVec2 offset = ImVec2(0.0, 0.0));
  
  // Redesign
  
  static void PushRedesignStyle();
  static void PopRedesignStyle();
  static void PushNodeRedesignStyle();
  static void PopNodeRedesignStyle();
  
  
  static bool SelectorTitleButton(std::string title, std::string id, float width);
  
  static void DownChevron(ImVec2 pos, float scale);
  
  static void NodePin(std::shared_ptr<Node> node, bool active);
  static ed::PinId InputNodePin(std::shared_ptr<Node> node, InputSlot slot);
  static ed::PinId OutputNodePin(std::shared_ptr<Node> node, OutputSlot slot);
  
  static void ImageNamedNew(std::string name, float width, float height);
  static void ImageNamedNew(std::string name, ImVec2 size);
  
  static bool ImageButton(std::string id, std::string imageName, ImVec2 size = ImVec2(105.0, 50.0), ImVec2 imageRatio = ImVec2(3.0, 2.0), bool forceImGuiContext = false);
  
  static bool TitledButton(std::shared_ptr<Parameter> param, std::string title, ImVec2 size = ImVec2(105.0, 50.0));
  
  static bool SmallImageButton(std::string id, std::string imageName);

  static bool SimpleImageButton(std::string id, std::string imageName, ImVec2 size = ImVec2(20.0, 20.0));

  static bool ShaderDropdownButton(std::shared_ptr<Parameter> param);

  static bool FlipButton(std::string id, std::shared_ptr<Parameter> param, ImVec2 size = ImVec2(30.0, 30.0));

  static bool PlayPauseButton(std::string id, bool playing, ImVec2 size = ImVec2(40.0, 40.0), ImVec2 padding = ImVec2(7.5, 7.5));

  static bool SearchBar(std::string& searchQuery, bool& searchDirty, std::string id);

  static void CollapsibleSearchHeader(
    const char* collapsedTitle,
    const char* searchPlaceholder,
    bool* collapsed,
    std::string& searchQuery,
    bool& searchDirty,
    std::string clearButtonId
  );
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
