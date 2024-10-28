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
#include "FavoriteParameter.hpp"
#include "ParameterTileBrowserView.hpp"
#include "Colors.hpp"
#include "Fonts.hpp"
#include "MidiService.hpp"
#include "ModulationService.hpp"
#include "OscillationService.hpp"
#include "ofMain.h"
#include "OscillatorView.hpp"
#include "Strings.hpp"
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

bool CommonViews::ShaderParameter(std::shared_ptr<Parameter> param,
                                  std::shared_ptr<Oscillator> osc)
{
  xsSpacing();
  H4Title(param->name);
  bool ret = Slider(param->name, param->paramId, param);
  MidiSelector(param);
  ImGui::SameLine();
  AudioParameterSelector(param);
  ImGui::SameLine();
  FavoriteButton(param);
  if (osc == nullptr)
    return ret;
  ImGui::SameLine();
  OscillateButton(param->paramId, osc, param);
  sSpacing();
  if (!osc->enabled->boolValue)
  {
    return ret;
  }
  OscillatorWindow(osc, param);
  return ret;
}

bool CommonViews::MiniSlider(std::shared_ptr<Parameter> param) {
  ImGui::Text("%s", param->name.c_str());
  ImGui::PushItemWidth(70.0);
  return ImGui::SliderFloat(idString(param->paramId).c_str(), &param->value, param->min, param->max);
}

bool CommonViews::MiniVSlider(std::shared_ptr<Parameter> param) {
  ImGui::Text("%s", param->name.c_str());
  return ImGui::VSliderFloat(idString(param->paramId).c_str(), ImVec2(10.0, 30.0), &param->value, param->min, param->max);
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
    ImGui::SliderFloat(formatString("Shift##%s", param->paramId.c_str()).c_str(), &param->shift->value, param->shift->min, param->shift->max, "%.3f", ImGuiSliderFlags_Logarithmic);
    ImGui::SliderFloat(formatString("Scale##%s", param->paramId.c_str()).c_str(), &param->scale->value, param->scale->min, param->scale->max, "%.3f", ImGuiSliderFlags_Logarithmic);
    return;
  }
  
  // Otherwise, present a button to select an audio parameter
  if (CommonViews::IconButton(ICON_MD_AUDIOTRACK, formatString("Audio Param##%s", param->name.c_str()).c_str()))
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

bool CommonViews::ShaderCheckbox(std::shared_ptr<Parameter> param, bool sameLine)
{
  if (!sameLine) sSpacing();
  
  bool old = param->boolValue;
  ImGui::Checkbox(param->name.c_str(), &param->boolValue);
  bool ret = false;
  if (old != param->boolValue)
  {
    ret = true;
    param->setValue(static_cast<float>(param->boolValue));
  }
  ImGui::SameLine();
  CommonViews::MidiSelector(param);
  ImGui::SameLine();
  CommonViews::FavoriteButton(param);
  if (!sameLine) sSpacing();
  return ret;
}

void CommonViews::H1Title(std::string title, bool padding)
{
  if (padding)
    CommonViews::Spacing(1);
  ImGui::PushFont(FontService::getService()->h1);
  ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(5.0, 5.0));
  ImGui::Text("%s", title.c_str());
  if (padding)
    CommonViews::Spacing(1);
  ImGui::PopStyleVar();
  ImGui::PopFont();
}

void CommonViews::H2Title(std::string title, bool padding)
{
  if (padding)
    CommonViews::Spacing(1);
  ImGui::PushFont(FontService::getService()->h2);
  ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(5.0, 5.0));
  ImGui::TextWrapped("%s", title.c_str());
  if (padding)
    CommonViews::Spacing(1);
  ImGui::PopStyleVar();
  ImGui::PopFont();
}

void CommonViews::H3Title(std::string title, bool padding)
{
  if (padding)
    CommonViews::Spacing(1);
  ImGui::PushFont(FontService::getService()->h3);
  ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(5.0, 5.0));
  ImGui::TextWrapped("%s", title.c_str());
  if (padding)
    CommonViews::Spacing(1);
  ImGui::PopStyleVar();
  ImGui::PopFont();
}

void CommonViews::H3BTitle(std::string title, bool padding)
{
  if (padding)
    CommonViews::Spacing(1);
  ImGui::PushFont(FontService::getService()->h3b);
  ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(5.0, 5.0));
  ImGui::TextWrapped("%s", title.c_str());
  if (padding)
    CommonViews::Spacing(1);
  ImGui::PopStyleVar();
  ImGui::PopFont();
}

void CommonViews::H4Title(std::string title, bool padding)
{
  if (padding)
    CommonViews::Spacing(1);
  ImGui::PushFont(FontService::getService()->h4);
  ImGui::TextWrapped("%s", title.c_str());
  if (padding)
    CommonViews::Spacing(1);
  ImGui::PopFont();
}

bool CommonViews::Slider(std::string title, std::string id,
                         std::shared_ptr<Parameter> param,
                         float width)
{
  ImGui::SetNextItemWidth(width);
  bool ret = ImGui::SliderFloat(idString(id).c_str(), &param->value, param->min, param->max, "%.3f");
  if (ret) {
    param->affirmValue();
  }
  ImGui::SameLine(0, 20);
  ResetButton(id, param);
  return ret;
}

void CommonViews::ColorShaderStageParameter(std::shared_ptr<Parameter> param) {
  H3Title(param->name);
  H4Title(param->ownerName);
  //  ImGui::SetCursorPosY(ImGui::GetCursorPosY() + ImGui::GetContentRegionAvail().y / 2.0);
  ImGui::ColorEdit4(idString(param->paramId).c_str(), param->color->data());
  // BUTTONS
  ImGui::SetCursorPosY(ImGui::GetCursorPosY() + ImGui::GetContentRegionAvail().y - 25);
  ResetButton(param->paramId, param);
  ImGui::SameLine();
  FavoriteButton(param);
}

void CommonViews::CheckboxShaderStageParameter(std::shared_ptr<Parameter> param) {
  H3Title(param->name);
  H4Title(param->ownerName);
  ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x / 2.0, ImGui::GetCursorPosY() + ImGui::GetContentRegionAvail().y / 2.0));
  CommonViews::ShaderCheckbox(param);
  // BUTTONS
  ImGui::SetCursorPosY(ImGui::GetCursorPosY() + ImGui::GetContentRegionAvail().y - 25);
  ResetButton(param->paramId, param);
  ImGui::SameLine();
  FavoriteButton(param);
}

void CommonViews::IntShaderStageParameter(std::shared_ptr<Parameter> param) {
  ImGui::Columns(2, formatString("%s_param_columns", param->paramId.c_str()).c_str(), ImGuiOldColumnFlags_NoResize | ImGuiOldColumnFlags_NoBorder);
  ImGui::SetColumnWidth(0, 65);
  ImGui::SetColumnWidth(1, 135);
  ImGui::PushStyleColor(ImGuiCol_SliderGrab, White90.Value);
  ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 3);
  bool ret = ImGui::VSliderInt(idString(param->paramId).c_str(), ImVec2(20, ImGui::GetContentRegionAvail().y - 10), &param->intValue, param->min, param->max, "");
  if (ret) {
    param->affirmIntValue();
  }
  ImGui::PopStyleColor();
  ImGui::SameLine();
  ImGui::Text("%s", formatString("%0.2f", param->max).c_str());
  ImGui::SetCursorPosY(ImGui::GetCursorPosY() + ImGui::GetContentRegionAvail().y - 20);
  ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 22);
  ImGui::Text("%s", formatString("%0.2f", param->min).c_str());
  ImGui::NextColumn();
  
  // TITLE
  auto yPos = ImGui::GetCursorPosY();
  H3Title(param->name);
  auto xPos = ImGui::GetCursorPosX();
  ImGui::SameLine();
  ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x - 25, yPos));
  
  if (LayoutStateService::getService()->allParametersInStageModeEnabled && IconButton(ICON_MD_CLOSE, formatString("%s_close_button", param->paramId.c_str()).c_str())) {
    ParameterService::getService()->removeFavoriteParameter(param);
  }
  ImGui::SetCursorPosX(xPos);
  H4Title(param->ownerName);
  
  // VALUE
  ImGui::SetCursorPosY(ImGui::GetCursorPosY() + ImGui::GetContentRegionAvail().y - 125);
  
  if (!param->options.empty()) {
    H2Title(param->options[param->intValue].c_str());
  } else {
    H2Title(formatString("%0.2f", param->value));
  }
  
  // BUTTONS
  ImGui::SetCursorPosY(ImGui::GetCursorPosY() + ImGui::GetContentRegionAvail().y - 25);
  ResetButton(param->paramId, param);
  ImGui::SameLine();
  FavoriteButton(param);

}


void CommonViews::ShaderStageParameter(std::shared_ptr<Parameter> param, std::shared_ptr<Oscillator> osc) {
  ImGui::Columns(2, formatString("%s_param_columns", param->paramId.c_str()).c_str(), ImGuiOldColumnFlags_NoResize | ImGuiOldColumnFlags_NoBorder);
  ImGui::SetColumnWidth(0, 65);
  ImGui::SetColumnWidth(1, 135);
  ImGui::PushStyleColor(ImGuiCol_SliderGrab, White90.Value);
  ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 3);
  bool ret = ImGui::VSliderFloat(idString(param->paramId).c_str(), ImVec2(20, ImGui::GetContentRegionAvail().y - 10), &param->value, param->min, param->max, "");
  if (ret) {
    param->affirmValue();
  }
  ImGui::SetItemUsingMouseWheel();
  if (ImGui::IsItemHovered()) {
    
    float wheel = ImGui::GetIO().MouseWheel;
    if (wheel)
    {
      if(ImGui::IsItemActive())
      {
        ImGui::ClearActiveID();
      }
      else
      {
        param->value += wheel * 0.3;
      }
    }
  }
  ImGui::PopStyleColor();
  ImGui::SameLine();
  ImGui::Text("%s", formatString("%0.2f", param->max).c_str());
  ImGui::SetCursorPosY(ImGui::GetCursorPosY() + ImGui::GetContentRegionAvail().y - 20);
  ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 22);
  ImGui::Text("%s", formatString("%0.2f", param->min).c_str());
  ImGui::NextColumn();
  
  // TITLE
  auto yPos = ImGui::GetCursorPosY();
  H3Title(param->name);
  auto xPos = ImGui::GetCursorPosX();
  ImGui::SameLine();
  ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x - 25, yPos));
  if (!LayoutStateService::getService()->allParametersInStageModeEnabled && IconButton(ICON_MD_CLOSE, formatString("%s_close_button", param->paramId.c_str()).c_str())) {
    ParameterService::getService()->removeFavoriteParameter(param);
  }
  ImGui::SetCursorPosX(xPos);
  H4Title(param->ownerName);
  
  // VALUE
  ImGui::SetCursorPosY(ImGui::GetCursorPosY() + ImGui::GetContentRegionAvail().y - 125);
  H2Title(formatString("%0.2f", param->value));
  
  // OSCILLATOR
  
  if (osc != nullptr && osc->enabled->boolValue) {
    OscillatorView::drawMini(osc, param);
  }
  
  // BUTTONS
  ImGui::SetCursorPosY(ImGui::GetCursorPosY() + ImGui::GetContentRegionAvail().y - 25);
  ResetButton(param->paramId, param);
  ImGui::SameLine();
  OscillateButton(param->paramId, osc, param);
  ImGui::SameLine();
  FavoriteButton(param);
  ImGui::Columns(1, formatString("%s_param_columns", param->paramId.c_str()).c_str());
}

bool CommonViews::AreaSlider(std::string id,
                             std::shared_ptr<Parameter> minX, std::shared_ptr<Parameter> maxX,
                             std::shared_ptr<Parameter> minY, std::shared_ptr<Parameter> maxY,
                             std::shared_ptr<Oscillator> minXOscillator, std::shared_ptr<Oscillator> maxXOscillator,
                             std::shared_ptr<Oscillator> minYOscillator, std::shared_ptr<Oscillator> maxYOscillator)  {
  
  static ImVec2 selectionCanvas = ImVec2(256.0, 144.0);
  
  ImVec2 start = ImVec2(minX->value, minY->value);
  ImVec2 end = ImVec2(maxX->value, maxY->value);
  auto screenPos = ImGui::GetCursorScreenPos() + ImVec2(200.0, 100.0);
  auto endPos = screenPos + selectionCanvas;
  
  ImDrawList* draw_list = ImGui::GetForegroundDrawList();
  // Full canvas
  draw_list->AddRectFilled(ImGui::GetCursorScreenPos(), ImGui::GetCursorScreenPos() + selectionCanvas, ImGui::GetColorU32(IM_COL32(0, 130, 216, 50)));
  
  // Selection window
  draw_list->AddRectFilled(ImGui::GetCursorScreenPos() + ImVec2(start.x * selectionCanvas.x, start.y * selectionCanvas.y), ImGui::GetCursorScreenPos() + ImVec2(end.x * selectionCanvas.x, end.y * selectionCanvas.y), ImGui::GetColorU32(IM_COL32(215, 30, 0, 100)));
  
  ImGui::SetCursorPosY(ImGui::GetCursorPosY() + selectionCanvas.y);
  
  if (IsMouseWithin(screenPos, endPos, ImVec2(200.0, 100.0)) && ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
    
    // Call the selection rectangle and handle interactions.
    bool selectionModified = CommonViews::SelectionRect(&start, &end, screenPos, endPos);
    
    ImVec2 size = ImVec2(endPos.x - screenPos.x, endPos.y - screenPos.y);
    
    // Mapping back the normalized coordinates to parameter space.
    minX->value = start.x;
    maxX->value = end.x;
    minY->value = start.y;
    maxY->value = end.y;
    ImGui::Text("%s", formatString("(%.2f, %.2f)", minX->value, minY->value).c_str());
    ImGui::Text("%s", formatString("(%.2f, %.2f)", maxX->value, maxY->value).c_str());
    
    return selectionModified;
  }
  
  return false;
}

bool CommonViews::SelectionRect(ImVec2 *start_pos, ImVec2 *end_pos, ImVec2 topLeft, ImVec2 bottomRight) {
  IM_ASSERT(start_pos != NULL);
  IM_ASSERT(end_pos != NULL);
  
  ImVec2 size = ImVec2(bottomRight.x - topLeft.x, bottomRight.y - topLeft.y);
  
  if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
    ImVec2 mousePos = getScaledMouseLocation() + ImVec2(200.0, 100.0);
    start_pos->x = (mousePos.x - topLeft.x) / size.x;
    start_pos->y = (mousePos.y - topLeft.y) / size.y;
  }
  
  if (ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
    ImVec2 mousePos = getScaledMouseLocation() + ImVec2(200.0, 100.0);
    end_pos->x = (mousePos.x - topLeft.x) / size.x;
    end_pos->y = (mousePos.y - topLeft.y) / size.y;
    
    ImDrawList* draw_list = ImGui::GetForegroundDrawList();
    ImVec2 actual_start_pos = ImVec2(topLeft.x + start_pos->x * size.x, topLeft.y + start_pos->y * size.y) - ImVec2(200.0, 100.0);;
    ImVec2 actual_end_pos = ImVec2(topLeft.x + end_pos->x * size.x, topLeft.y + end_pos->y * size.y) - ImVec2(200.0, 100.0);;
    
    draw_list->AddRect(actual_start_pos, actual_end_pos, ImGui::GetColorU32(IM_COL32(0, 130, 216, 255))); // Border
    draw_list->AddRectFilled(actual_start_pos, actual_end_pos, ImGui::GetColorU32(IM_COL32(0, 130, 216, 50))); // Background
  }
  
  return ImGui::IsMouseReleased(ImGuiMouseButton_Left);
}

void CommonViews::MultiSlider(std::string title, std::string id, std::shared_ptr<Parameter> param1, std::shared_ptr<Parameter> param2,
                              std::shared_ptr<Oscillator> param1Oscillator,
                              std::shared_ptr<Oscillator> param2Oscillator) {
  ImGui::PushItemWidth(400);
  CommonViews::H4Title(title);
  ImGui::Columns(2);
  ImGui::SetColumnWidth(0, 200);
  ImGui::SetColumnWidth(1, 200);
  ImGuiExtensions::Slider2DFloat("", &param1->value,
                                 &param2->value,
                                 param1->min, param1->max, param2->min, param2->max, 1.0);
  ImGui::NextColumn();
  ImGui::Text("X Translate");
  CommonViews::ResetButton("##xMultiSliderReset", param1);
  ImGui::SameLine();
  MidiSelector(param1);
  ImGui::SameLine();
  AudioParameterSelector(param1);
  ImGui::SameLine();
  FavoriteButton(param1);
  ImGui::SameLine();
  CommonViews::OscillateButton(formatString("##xOscillator_%s", id.c_str()).c_str(), param1Oscillator, param1);
  CommonViews::Slider(param1->name, param1->paramId, param1, 150.0);
  ImGui::Text("Y Translate");
  CommonViews::ResetButton("##yMultiSliderReset", param2);
  ImGui::SameLine();
  MidiSelector(param2);
  ImGui::SameLine();
  AudioParameterSelector(param2);
  ImGui::SameLine();
  FavoriteButton(param2);
  ImGui::SameLine();
  CommonViews::OscillateButton(formatString("##yOscillator_%s", id.c_str()).c_str(), param2Oscillator, param2);
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
    if (ImGui::Begin(idString(param->name).c_str(), 0, ImGuiWindowFlags_NoDecoration)) {
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
    auto title = formatString("##%s", param->name.c_str());
    if (ImGui::Begin(title.c_str()), ImGuiWindowFlags_NoTitleBar) {
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
  H4Title(param->name);
  ImGui::SetNextItemWidth(200.0);
  ImGui::ColorEdit4(idString(param->paramId).c_str(), param->color->data());
  
  // Color history
  if (IconButton(ICON_MD_SAVE, formatString("##%ssaveColor", param->paramId.c_str()).c_str())) {
    LayoutStateService::getService()->pushColor(param->color);
  }
  ImGui::SameLine(0, 20);
  
  std::vector<std::array<float, 4>> colors = LayoutStateService::getService()->colorHistory;
  auto items = mapColorsToStrings(colors);
  auto preview = formatColor(*param->color.get());
  if (ImGui::BeginCombo(formatString("Color History##%sColorHistory", param->paramId.c_str()).c_str(), preview.c_str(), 0))
  {
    for (int n = 0; n < items.size(); n++)
    {
      std::array<float, 4> color = colors[n];
      ImGui::PushStyleColor(ImGuiCol_Text, ImColor(color[0], color[1], color[2], color[3]).Value);
      if (ImGui::Selectable(items[n].c_str(), false)) {
        param->setColor(colors[n]);
      }
      ImGui::PopStyleColor();
    }
    ImGui::EndCombo();
  }
  ImGui::SameLine(0, 20);
  GLubyte *pixels = new GLubyte[640*480*4];
  if (IconButton(ICON_FA_EYE_DROPPER, idString(param->paramId).c_str())) {
    LayoutStateService::getService()->isEyeDroppingColor = true;
    glReadPixels(200,200,1,1,GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    log("%s, %s, %s", pixels[0], pixels[1], pixels[2]);
  }
  mSpacing();
}

void CommonViews::ImageNamed(std::string filename, double width, double height) {
  static std::map<std::string, ofImage> imageMap;
  
  if (imageMap.count(filename) == 0) {
    ofImage image = ofImage();
    image.load("images/icons/" + filename);
    if (!image.isAllocated()) {
      ofLogError("Main") << "Image not correctly loaded!";
      return;
    }
    imageMap[filename] = image;
  }
  ofImage image = imageMap[filename];
  image.draw(getScaledCursorScreenLocation().x, getScaledCursorScreenLocation().y, scaleFloat(width), scaleFloat(height));
}

void CommonViews::PlaybackRangeSlider(std::string title, std::string id, std::shared_ptr<Parameter> param, std::shared_ptr<Parameter> param2, float duration, bool dirty) {
  auto startTime = formatTimeDuration(duration * param->value);
  auto endTime = formatTimeDuration(duration * param2->value);
  auto formattedLabel = formatString("%s : %s", startTime.c_str(), endTime.c_str());
  
  RangeSlider(title, id, param, param2, formattedLabel, dirty);
}

void CommonViews::RangeSlider(std::string title, std::string id, std::shared_ptr<Parameter> param, std::shared_ptr<Parameter> param2, std::string label, bool dirty)
{
  if (dirty) {
    ImGui::PushStyleColor(ImGuiCol_SliderGrab, WarningColor.Value);
  }
  
  ImGui::SetNextItemWidth(300.0);
  ImGui::RangeSliderFloat(title.c_str(), &param->value, &param2->value, fmin(param->min, param2->min), fmax(param->max, param2->max), label.c_str(), 1.0);
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

void CommonViews::ShaderIntParameter(std::shared_ptr<Parameter> param)
{
  H4Title(param->name);
  ImGui::SetNextItemWidth(200.0);
  if (ImGui::SliderInt(formatString("##%s", param->paramId.c_str()).c_str(), &param->intValue, param->min, param->max, "%d")) {
    param->setValue(static_cast<float>(param->intValue));
  }
}

bool CommonViews::ResetButton(std::string id,
                              std::shared_ptr<Parameter> param)
{
  bool hit = IconButton(ICON_MD_RESTORE,   formatString(" ##%s_reset_button", id.c_str()).c_str());
  
  if (hit)
  {
    param->resetValue();
  }
  return hit;
}

bool CommonViews::ShaderOption(std::shared_ptr<Parameter> param, std::vector<std::string> options, bool drawTitle) {
  if (options.empty()) return false;
  
  if (drawTitle) {
    ImGui::Text("%s", param->name.c_str());
    ImGui::SameLine(0, 20);
  }
  
  // Convert vector of std::string to vector of const char* for ImGui::Combo
  std::vector<const char*> items;
  for (const auto& option : options) {
    items.push_back(option.c_str());
  }
  
  int currentItem = static_cast<int>(param->value); // Assuming param->value holds the index of the current selected option
  ImGui::PushID(param->paramId.c_str());
  if (ImGui::Combo("", &currentItem, items.data(), items.size())) {
    param->intValue = currentItem;
    param->affirmIntValue();
    ImGui::PopID();
    return true;
  }
  ImGui::PopID();
  return false;
}

bool CommonViews::InvisibleIconButton(std::string id)
{
  return ImGui::InvisibleButton(id.c_str(), ImVec2(16., 16.));
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

void CommonViews::XLargeIconTitle(const char *icon)
{
  auto buttonId = formatString("%s", icon);
  ImGui::PushFont(FontService::getService()->xLargeIcon);
  ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(24.0, 24.0));
  ImGui::Text(buttonId.c_str());
  ImGui::PopStyleVar();
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

bool CommonViews::XLargeIconButton(const char *icon, std::string id)
{
  auto buttonId = formatString("%s##%s", icon, id.c_str());
  ImGui::PushFont(FontService::getService()->xLargeIcon);
  ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32_BLACK_TRANS);
  ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0., 0.));
  auto button = ImGui::Button(buttonId.c_str(), ImVec2(100.0, 100.0));
  ImGui::PopStyleColor();
  ImGui::PopStyleVar();
  ImGui::PopFont();
  
  return button;
}

void CommonViews::OscillateButton(std::string id, std::shared_ptr<Oscillator> o,
                                  std::shared_ptr<Parameter> p)
{
  if (o == nullptr) { return; }
  ImGui::PushFont(FontService::getService()->icon);
  ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 0.0f));
  
  std::string buttonTitle;
  auto icon = o->enabled->boolValue ? ICON_MD_CLOSE : ICON_MD_WAVES;
  
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
    LayoutStateService::getService()->utilityPanelTab = 1;
  }
  ImGui::PopStyleVar();
  ImGui::PopFont();
}

void CommonViews::MidiSelector(std::shared_ptr<Parameter> videoParam)
{
  if (!LayoutStateService::getService()->midiEnabled) return;
  
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

void CommonViews::FavoriteButton(std::shared_ptr<Parameter> param) {
  if (param == nullptr) {
    return;
  }
  bool hasFavorite = ParameterService::getService()->hasFavoriteParameterFor(param);
  auto icon = hasFavorite ? ICON_MD_UNDO : ICON_MD_FAVORITE;
  if (IconButton(icon, param->paramId)) {
    if (!hasFavorite) {
      ParameterService::getService()->addFavoriteParameter(param);
    } else {
      ParameterService::getService()->removeFavoriteParameter(param);
    }
  }
}

void CommonViews::BlendModeSelector(std::shared_ptr<Parameter> blendMode, std::shared_ptr<Parameter> flip, std::shared_ptr<Parameter> alpha, std::shared_ptr<Oscillator> alphaOscillator, std::shared_ptr<Parameter> blendWithEmpty)
{
  static std::vector<std::string> BlendModeNames = {
    "Multiply",
    "Screen",
    "Darken",
    "Lighten",
    "Difference",
    "Exclusion",
    "Overlay",
    "Hard Light",
    "Soft Light",
    "Color Dodge",
    "Linear Dodge",
    "Burn",
    "Linear Burn"
  };
  
  // Draw a combo selector
  ImGui::Text("Blend Mode");
  ImGui::SameLine();
  ImGui::PushItemWidth(200);
  // Convert the strings to C strings
  std::vector<const char *> blendModeNamesC;
  for (auto &name : BlendModeNames) {
    blendModeNamesC.push_back(name.c_str());
  }
  ImGui::Combo("##BlendMode", &blendMode->intValue, blendModeNamesC.data(), (int) BlendModeNames.size());
  
  if (flip != nullptr) {
    ImGui::SameLine();
    CommonViews::ShaderCheckbox(flip, true);
  }
  if (alpha != nullptr) {
    CommonViews::ShaderParameter(alpha, alphaOscillator);
  }
  
  if (blendWithEmpty != nullptr) {
    CommonViews::ShaderCheckbox(blendWithEmpty);
  }
}

bool CommonViews::Selector(std::shared_ptr<Parameter> param, std::vector<std::string> options)
{
  ImGui::Text("%s", param->name.c_str());
  ImGui::SameLine();
  ImGui::PushItemWidth(200);
  // Convert the strings to C strings
  std::vector<const char *> optionsC;
  for (auto &option : options) {
    optionsC.push_back(option.c_str());
  }
  return ImGui::Combo(formatString("%s", param->name.c_str()).c_str(), &param->intValue, optionsC.data(), (int) options.size());
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

bool CommonViews::IsMouseWithin(ImVec2 topLeft, ImVec2 bottomRight, ImVec2 offset) {
  auto mousePos = ImGui::GetMousePos();
  mousePos += offset;
  return mousePos.x >= topLeft.x && mousePos.x <= bottomRight.x &&
  mousePos.y >= topLeft.y && mousePos.y <= bottomRight.y;
}

void CommonViews::PaddedText(std::string text, ImVec2 padding)
{
    // Calculate the size of the text with padding
    ImVec2 textSize = ImGui::CalcTextSize(text.c_str());
    ImVec2 paddedSize = ImVec2(textSize.x + padding.x * 2, textSize.y + padding.y * 2);

    // Save the original cursor position
    ImVec2 originalCursorPos = ImGui::GetCursorPos();

    // Set the cursor position to account for padding
    ImGui::SetCursorPos(originalCursorPos + padding);

    // Draw the text
    ImGui::TextUnformatted(text.c_str());

    // Restore the cursor position to the original position + padded size to allow for correct layout
    ImGui::SetCursorPos(originalCursorPos + ImVec2(0, paddedSize.y));
}
