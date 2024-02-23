//
//  CommonViews.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 2022-05-12.
//

#include "LayoutStateService.hpp"
#include "CommonViews.hpp"
#include "AudioSourceService.hpp"
#include "Icon.hpp"
#include "IconService.hpp"
#include "TextureService.hpp"
#include "FontService.hpp"
#include "ImGuiExtensions.hpp"
#include "ParameterTileBrowserView.hpp"
#include "Colors.hpp"
#include "Fonts.hpp"
#include "MidiService.hpp"
#include "ModulationService.hpp"
#include "OscillationService.hpp"
#include "ofMain.h"
#include "OscillatorView.hpp"
#include "Strings.hpp"
#include "TextureBrowserView.hpp"
#include "imgui.h"
#include <imgui_node_editor.h>
#include <imgui_node_editor_internal.h>

namespace ed = ax::NodeEditor;


void CommonViews::xsSpacing() { Spacing(4); }

void CommonViews::sSpacing() { Spacing(8); }

void CommonViews::mSpacing() { Spacing(16); }

void CommonViews::lSpacing() { Spacing(24); }

void CommonViews::xlSpacing() { Spacing(36); }

void CommonViews::blankSpacing(int n)
{
  ImGui::PushStyleColor(ImGuiCol_PopupBg, ImGui::GetColorU32(ImVec4(0, 0, 0, 0)));
  Spacing(n);
  ImGui::PopStyleColor();
}

void CommonViews::Spacing(int n)
{
  for (int i = 0; i < n; i++)
  {
    ImGui::Spacing();
  }
}

void CommonViews::HSpacing(int n)
{
  for (int i = 0; i < n; i++)
  {
    ImGui::Spacing();
    ImGui::SameLine();
  }
}

void CommonViews::ShaderParameter(std::shared_ptr<Parameter> param,
                                  std::shared_ptr<Oscillator> osc)
{
  xsSpacing();
  H4Title(param->name);
  Slider(param->name, param->paramId, param);
  MidiSelector(param);
  ImGui::SameLine();
  AudioParameterSelector(param);
  if (osc == nullptr)
    return;
  ImGui::SameLine();
  OscillateButton(param->paramId, osc, param);
  sSpacing();
  if (!osc->enabled->boolValue)
  {
    return;
  }
  OscillatorWindow(osc, param);
}

void CommonViews::OscillatorWindow(std::shared_ptr<Oscillator> o, std::shared_ptr<Parameter> p)
{
  auto pos = ImGui::GetCursorScreenPos();
  ImGui::SetNextWindowPos(pos);

  if (ImGui::Begin(p->oscPopupId().c_str(), 0, ImGuiWindowFlags_AlwaysAutoResize))
  {
    OscillatorView::draw(o, p);
  }
  ImGui::End();
}

void CommonViews::AudioParameterSelector(std::shared_ptr<Parameter> param)
{
  if (AudioSourceService::getService()->selectedAudioSource == nullptr)
    return;

  auto analysisParameters = AudioSourceService::getService()
                                ->selectedAudioSource->audioAnalysis.parameters;
  auto buttonId = formatString("##audio_button_%s", param->name.c_str());

  // If the parameter is already mapped to an audio parameter, show the name of
  // the audio parameter and a button to remove the mapping
  if (param->driver != NULL)
  {
    if (ImGui::Button(param->driver->name.c_str()))
    {
      param->driver = NULL;
    }
    ImGui::SliderFloat(formatString("Shift##%s", param->paramId.c_str()).c_str(), &param->shift->value, param->shift->min, param->shift->max);
    ImGui::SliderFloat(formatString("Scale##%s", param->paramId.c_str()).c_str(), &param->scale->value, param->scale->min, param->scale->max);
    return;
  }

  // Otherwise, present a button to select an audio parameter
  if (ImGui::Button(formatString("Audio Param##%s", param->name.c_str()).c_str()))
  {
    ImGui::OpenPopup(param->audioPopupId().c_str());
  }

  // If the user selects an audio parameter, map it to the parameter
  if (ImGui::BeginPopup(param->audioPopupId().c_str()))
  {
    ImGui::Text("Select Audio Parameter to Follow");
    for (auto audioParam : analysisParameters)
    {
      if (ImGui::Selectable(audioParam->name.c_str()))
      {
        param->addDriver(audioParam);
      }
    }
    ImGui::EndPopup();
  }
}

void CommonViews::ResolutionSelector(std::shared_ptr<VideoSource> source)
{
  static const char *resolutions[] = {"240p", "360p", "480p", "720p", "1080p", "1440p", "4k"};

  auto comboId = formatString("##Resolution%s", source->id.c_str());
  auto selectedIdx = source->settings->resolution->intValue;
  ImGui::PushItemWidth(100.0);
  if (ImGui::BeginCombo(comboId.c_str(), resolutions[source->settings->resolution->intValue]))
  {
    for (int i = 0; i < IM_ARRAYSIZE(resolutions); i++)
    {
      bool isSelected = (selectedIdx == i);
      if (ImGui::Selectable(resolutions[i], isSelected))
      {
        source->settings->resolution->intValue = i;
        source->settings->updateResolutionSettings();
      }
      if (isSelected)
        ImGui::SetItemDefaultFocus();
    }
    ImGui::EndCombo();
  }
}

bool CommonViews::ShaderCheckbox(std::shared_ptr<Parameter> param)
{
  sSpacing();
  bool old = param->boolValue;
  ImGui::Checkbox(param->name.c_str(), &param->boolValue);
  bool ret = false;
  if (old != param->boolValue)
  {
    ret = true;
    param->setValue(static_cast<float>(param->boolValue));
  }
  sSpacing();
  return ret;
}

void CommonViews::H3Title(std::string title)
{
  CommonViews::Spacing(1);
  ImGui::PushFont(FontService::getService()->h3);
  ImGui::Text("%s", title.c_str());
  CommonViews::Spacing(1);
  ImGui::PopFont();
}

void CommonViews::H4Title(std::string title)
{
  CommonViews::Spacing(1);
  ImGui::PushFont(FontService::getService()->h4);
  ImGui::Text("%s", title.c_str());
  CommonViews::Spacing(1);
  ImGui::PopFont();
}

bool CommonViews::Slider(std::string title, std::string id,
                         std::shared_ptr<Parameter> param)
{
  ImGui::SetNextItemWidth(200.0);
  bool ret = ImGui::SliderFloat(idString(id).c_str(), &param->value, param->min, param->max, "%.3f");
  if (ret) {
    param->affirmValue();
  }
  ImGui::SameLine(0, 20);
  ResetButton(id, param);
  return ret;
}

bool CommonViews::MultiSlider(std::string title, std::string id, std::shared_ptr<Parameter> param1, std::shared_ptr<Parameter> param2,
                              std::shared_ptr<Oscillator> param1Oscillator,
                              std::shared_ptr<Oscillator> param2Oscillator) {
  CommonViews::H4Title(title);
  ImGui::Columns(2);
  ImGui::SetColumnWidth(0, 200);
  ImGuiExtensions::Slider2DFloat("", &param1->value,
                                 &param2->value,
                                 -1, 1., -1, 1., 1.0);
  ImGui::NextColumn();
  ImGui::Text("X Translate");
  ImGui::SameLine();
  CommonViews::OscillateButton("##xOscillator", param1Oscillator, param1);
  ImGui::SameLine();
  CommonViews::ResetButton("##xMultiSliderReset", param1);
  ImGui::Text("Y Translate");
  ImGui::SameLine();
  CommonViews::OscillateButton("##yOscillator", param2Oscillator, param2);
  ImGui::SameLine();
  CommonViews::ResetButton("##yMultiSliderReset", param2);
  CommonViews::Slider(param1->name, param1->paramId, param1);
  CommonViews::Slider(param2->name, param2->paramId, param2);
  if (param1Oscillator->enabled->boolValue)
  {
    OscillatorWindow(param1Oscillator, param1);
  }
  
  if (param2Oscillator->enabled->boolValue)
  {
    OscillatorWindow(param2Oscillator, param2);
  }
  ImGui::Columns(1);
}

bool CommonViews::TextureFieldAndBrowser(std::shared_ptr<Parameter> param) {
  std::vector<std::shared_ptr<Texture>> textures = TextureService::getService()->availableTextures();
  std::vector<std::shared_ptr<ParameterTileItem>> items;
  
  for (int i = 0; i < textures.size(); i++)
  {
    auto texture = textures[i];
    items.push_back(std::make_shared<ParameterTileItem>(texture->name,  (ImTextureID)(uintptr_t) texture->fbo.getTexture().getTextureData().textureID, i));
  }
  
  std::string currentTextureName = textures[param->intValue]->name;
  ImGui::Text("Current Texture: ");
  ImGui::SameLine();
  static std::map<std::string, bool> openMap;
  if (openMap.count(param->paramId) == 0) {
    openMap[param->paramId] = false;
  }
  bool ret = false;
  
  if (ImGui::Button(currentTextureName.c_str())) {
    openMap[param->paramId] = !openMap[param->paramId];
    if (openMap[param->paramId]) {
      ImGui::SetNextWindowPos(ImGui::GetMousePos());
      ImGui::SetNextWindowSize(ImVec2(400, 300));
    }
  }
  if (openMap[param->paramId]) {
    ImGui::SameLine();
    ImGui::SetNextWindowPos(ImGui::GetCursorScreenPos());
    if (ImGui::Begin(idString(param->name).c_str()), ImGuiWindowFlags_NoDecoration) {
      ret = ParameterTileBrowserView::draw(items, param, false);
    }
    ImGui::End();
    ImGui::NewLine();
  }
  if (ret) {
    ImGui::CloseCurrentPopup();
  }
  return ret;
}

bool CommonViews::IconFieldAndBrowser(std::shared_ptr<Parameter> param) {
  std::vector<std::shared_ptr<Icon>> icons = IconService::getService()->availableIcons();
  std::vector<std::shared_ptr<ParameterTileItem>> items;
  
  for (int i = 0; i < icons.size(); i++)
  {
    auto icon = icons[i];
    items.push_back(std::make_shared<ParameterTileItem>(icon->name,  (ImTextureID)(uintptr_t) icon->fbo.getTexture().getTextureData().textureID, i, icon->category));
  }
  
  std::string currentTextureName = icons[param->intValue]->name;
  ImGui::Text("Current Icon: ");
  ImGui::SameLine();
  static std::map<std::string, bool> openMap;
  if (openMap.count(param->paramId) == 0) {
    openMap[param->paramId] = false;
  }
  bool ret = false;
  
  if (ImGui::Button(currentTextureName.c_str())) {
    openMap[param->paramId] = !openMap[param->paramId];
    if (openMap[param->paramId]) {
      ImGui::SetNextWindowPos(ImGui::GetMousePos());
      ImGui::SetNextWindowSize(ImVec2(400, 300));
    }
  }
  if (openMap[param->paramId]) {
    ImGui::SameLine();
    ImGui::SetNextWindowPos(ImGui::GetCursorScreenPos());
    auto title = formatString("##%s", param->name.c_str()).c_str();
    if (ImGui::Begin(title), ImGuiWindowFlags_NoTitleBar) {
      ret = ParameterTileBrowserView::draw(items, param, false);
    }
    ImGui::End();
    ImGui::NewLine();
  }
  if (ret) {
    ImGui::CloseCurrentPopup();
  }
  return ret;
}

void CommonViews::ShaderColor(std::shared_ptr<Parameter> param)
{
  ImGui::ColorEdit3(param->name.c_str(), param->color->data());
  
  // Color history
  if (IconButton(ICON_MD_SAVE, "##saveColor")) {
    LayoutStateService::getService()->pushColor(param->color);
  }
  ImGui::SameLine(0, 20);
  
  std::vector<std::array<float, 3>> colors = LayoutStateService::getService()->colorHistory;
  auto items = mapColorsToStrings(colors);
  auto preview = formatColor(*param->color.get());
  if (ImGui::BeginCombo(formatString("Color History##%sColorHistory", param->paramId.c_str()).c_str(), preview.c_str(), 0))
  {
      for (int n = 0; n < items.size(); n++)
      {
        std::array<float, 3> color = colors[n];
        ImGui::PushStyleColor(ImGuiCol_Text, ImColor(color[0], color[1], color[2]).Value);
        if (ImGui::Selectable(items[n].c_str(), false)) {
          param->setColor(colors[n]);
        }
        ImGui::PopStyleColor();
      }
      ImGui::EndCombo();
  }
}

void CommonViews::RangeSlider(std::string title, std::string id, std::shared_ptr<Parameter> param, std::shared_ptr<Parameter> param2, float duration, bool dirty)
{
  if (dirty) {
    ImGui::PushStyleColor(ImGuiCol_SliderGrab, WarningColor.Value);
  }
  ImGui::SetNextItemWidth(300.0);
  auto startTime = formatTimeDuration(duration * param->value);
  auto endTime = formatTimeDuration(duration * param2->value);
  auto formattedLabel = formatString("%s : %s", startTime.c_str(), endTime.c_str());
  ImGui::RangeSliderFloat(title.c_str(), &param->value, &param2->value, fmin(param->min, param2->min), fmax(param->max, param2->max), formattedLabel.c_str(), 1.0);
  if (dirty) {
    ImGui::PopStyleColor();
  }
}

// Draws a slider driving the param with a label representing the param's value (0..1)
// represented as a timestamp, with length being the total length in seconds.
// The timestamp should be converted to a string in the format "mm:ss"
void CommonViews::PlaybackSlider(std::shared_ptr<Parameter> param, float length)
{
  ImGui::SetNextItemWidth(300.0);
  auto timeString = formatString("%s : %s", formatTimeDuration(param->value * length).c_str(), formatTimeDuration(length).c_str());
  
  ImGui::SliderFloat(formatString("##%s", param->paramId.c_str()).c_str(),
                     &param->value,
                     0.0, 1.0,
                     timeString.c_str());
  ImGui::SameLine(0, 20);
  ResetButton(param->paramId, param);
}

void CommonViews::IntSlider(std::string title, std::string id,
                            std::shared_ptr<Parameter> param)
{
  ImGui::Text("%s", title.c_str());
  ImGui::SameLine(0, 20);
  ImGui::SetNextItemWidth(200.0);
  ImGui::SliderInt(id.c_str(), &param->intValue, param->min, param->max, "%d");
}

void CommonViews::ResetButton(std::string id,
                              std::shared_ptr<Parameter> param)
{
  ImGui::PushFont(FontService::getService()->icon);
  ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 0.0f));
  if (ImGui::Button(formatString(" ##%s_reset_button", id.c_str()).c_str(),
                    ImVec2(16.0, 16.0)))
  {
    param->resetValue();
  }
  ImGui::PopFont();
  ImGui::PopStyleVar();
}

bool CommonViews::ShaderOption(std::shared_ptr<Parameter> param, std::vector<std::string> options) {
    if (options.empty()) return; // Early exit if no options provided

    ImGui::Text("%s", param->name.c_str()); // Display the name of the parameter
    ImGui::SameLine(0, 20); // Align next item on the same line with spacing

    // Convert vector of std::string to vector of const char* for ImGui::Combo
    std::vector<const char*> items;
    for (const auto& option : options) {
        items.push_back(option.c_str());
    }

    int currentItem = static_cast<int>(param->value); // Assuming param->value holds the index of the current selected option
    if (ImGui::Combo(param->name.c_str(), &currentItem, items.data(), items.size())) {
        param->setValue(static_cast<float>(currentItem));
      return true;
    }
  return false;
}


bool CommonViews::IconButton(const char *icon, std::string id)
{
  auto buttonId = formatString("%s##%s", icon, id.c_str());
  ImGui::PushFont(FontService::getService()->icon);
  ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
  ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0., 0.));
  auto button = ImGui::Button(buttonId.c_str(), ImVec2(16., 16.));
  ImGui::PopStyleColor();
  ImGui::PopStyleVar();
  ImGui::PopFont();

  return button;
}

void CommonViews::IconTitle(const char *icon)
{
  ImGui::PushFont(FontService::getService()->icon);
  ImGui::Text(icon);
  ImGui::PopFont();
}

bool CommonViews::LargeIconButton(const char *icon, std::string id)
{
  auto buttonId = formatString("%s##%s", icon, id.c_str());
  ImGui::PushFont(FontService::getService()->largeIcon);
  ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32_BLACK_TRANS);
  ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0., 0.));
  auto button = ImGui::Button(buttonId.c_str(), ImVec2(24., 24.));
  ImGui::PopStyleColor();
  ImGui::PopStyleVar();
  ImGui::PopFont();

  return button;
}

void CommonViews::OscillateButton(std::string id, std::shared_ptr<Oscillator> o,
                                  std::shared_ptr<Parameter> p)
{
  ImGui::PushFont(FontService::getService()->icon);
  ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 0.0f));

  std::string buttonTitle;
  auto icon = o->enabled->boolValue ? ICON_MD_CLOSE : ICON_MD_SCATTER_PLOT;

  if (IconButton(icon, formatString("##%s_osc_button", id.c_str()).c_str()))
  {
    if (ImGui::IsPopupOpen(p->oscPopupId().c_str()))
    {
      ImGui::CloseCurrentPopup();
    }
    else
    {
      ImGui::OpenPopup(p->oscPopupId().c_str());
    }

    o->enabled->boolValue = !o->enabled->boolValue;
    OscillationService::getService()->selectOscillator(o, p);
  }
  ImGui::PopStyleVar();
  ImGui::PopFont();
}

void CommonViews::MidiSelector(std::shared_ptr<Parameter> videoParam)
{
  bool hasPairing =
      MidiService::getService()->hasPairingForParameterId(videoParam->paramId);
  bool enabled = false;

  std::function<void()> learnParamBlock = [videoParam]
  {
    MidiService::getService()->beginLearning(videoParam);
  };

  std::function<void()> stopLearningBlock = [videoParam]
  {
    // Currently learning the passed Param, so stop Learning
    MidiService::getService()->stopLearning();
  };

  std::string buttonTitle;
  std::function<void()> buttonAction;

  if (hasPairing)
  {
    // Already have a pairing, so reset it and start Learning
    buttonTitle = MidiService::getService()
                      ->pairingForParameterId(videoParam->paramId)
                      ->descriptor;
    buttonAction = learnParamBlock;
  }
  else if (MidiService::getService()->isLearning() &&
           MidiService::getService()->learningParam == videoParam)
  {
    // Currently learning the passed Param, so stop Learning
    buttonTitle = "Learning";
    buttonAction = stopLearningBlock;
  }
  else if (MidiService::getService()->isLearning())
  {
    // Currently learning a different Param, so start Learning this one
    buttonTitle = "Learning Other Parameter";
    buttonAction = learnParamBlock;
  }
  else
  {
    // Learn Assignment
    buttonTitle = "Learn Assignment";
    buttonAction = learnParamBlock;
  }

  auto idTitle =
      formatString("%s##%s", buttonTitle.c_str(), videoParam->name.c_str());

  if (ImGui::Button(idTitle.c_str()))
  {
    buttonAction();
  }
}

void CommonViews::HorizontallyAligned(float width, float alignment)
{
  ImGuiStyle &style = ImGui::GetStyle();
  float avail = ImGui::GetContentRegionAvail().x;
  float off = (avail - width) * alignment;
  if (off > 0.0f)
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + off);
}

void CommonViews::CenteredVerticalLine()
{
  HorizontallyAligned(1);
  ImGui::Text("|");
}

ImVec2 CommonViews::windowCanvasSize()
{
  return ImGui::GetWindowContentRegionMax();
}


