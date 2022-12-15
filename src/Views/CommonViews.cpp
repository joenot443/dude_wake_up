//
//  CommonViews.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 2022-05-12.
//

#include "CommonViews.hpp"
#include "Fonts.hpp"
#include "FontService.hpp"
#include "OscillatorView.hpp"
#include "ModulationService.hpp"
#include "OscillationService.hpp"
#include "MidiService.hpp"
#include "imgui.h"
#include "Strings.hpp"

void CommonViews::sSpacing() {
  Spacing(8);
}

void CommonViews::mSpacing() {
  Spacing(16);
}

void CommonViews::lSpacing() {
  Spacing(24);
}

void CommonViews::xlSpacing() {
  Spacing(36);
}


void CommonViews::Spacing(int n) {
  for (int i = 0; i < n; i++) {
    ImGui::Spacing();
  }
}

void CommonViews::ShaderParameter(std::shared_ptr<Parameter> param, std::shared_ptr<Oscillator> osc) {
  sSpacing();
  H4Title(param->name);
  Slider(param->name, param->paramId, param);
  MidiSelector(param);
  ImGui::SameLine();
  OscillateButton(param->paramId, osc, param);
  sSpacing();
}

void CommonViews::ShaderCheckbox(std::shared_ptr<Parameter> param) {
  sSpacing();
  H4Title(param->name);
  ImGui::Checkbox(param->name.c_str(), &param->boolValue);
  sSpacing();
}

void CommonViews::ModulationSelector(std::shared_ptr<Parameter> videoParam) { 
  return;
  
  bool hasDriver = videoParam->driver != NULL;
  auto popupId = formatString("##%s_popup", videoParam->name.c_str());
  auto buttonId = formatString("##mod_button_%s", videoParam->name.c_str());
  
  if (hasDriver) {
    if (ImGui::Button(videoParam->driver->name.c_str())) {
      ModulationService::getService()->removeMapping(videoParam);
    }
    return;
  }
  
  if (ImGui::Button(formatString("Select Audio Parameter%s", buttonId.c_str()).c_str())) {
    ImGui::OpenPopup(popupId.c_str());
  }
  
  
  if (ImGui::BeginPopup(popupId.c_str())) {
    for (auto const a : ModulationService::getService()->audioAnalysis) {
      if (ImGui::BeginMenu(a->name.c_str())) {
        for (auto const audioParam : a->parameters) {
          bool enabled = audioParam == videoParam->driver;
          if (ImGui::MenuItem(audioParam->name.c_str(), "", &enabled)) {
            ModulationService::getService()->addMapping(videoParam, audioParam);
          }
        }
        ImGui::EndMenu();
      }
    }
    ImGui::EndPopup();
  }
}

void CommonViews::H3Title(std::string title) {
  CommonViews::Spacing(2);
  ImGui::PushFont(FontService::getService()->h3);
  ImGui::Text("%s", title.c_str());
  CommonViews::Spacing(2);
  ImGui::PopFont();
}

void CommonViews::H4Title(std::string title) {
  CommonViews::Spacing(2);
  ImGui::PushFont(FontService::getService()->h4);
  ImGui::Text("%s", title.c_str());
  CommonViews::Spacing(2);
  ImGui::PopFont();
}

void CommonViews::Slider(std::string title, std::string id, std::shared_ptr<Parameter> param) {
  ImGui::SetNextItemWidth(200.0);
  ImGui::SliderFloat(idString(id).c_str(), &param->value, param->min, param->max, "%.3f");
  ImGui::SameLine(0, 20);
  ResetButton(id, param);
}

void CommonViews::IntSlider(std::string title, std::string id, std::shared_ptr<Parameter> param) {
  ImGui::Text("%s", title.c_str());
  ImGui::SameLine(0, 20);
  ImGui::SetNextItemWidth(200.0);
  ImGui::SliderInt(id.c_str(), &param->intValue, param->min, param->max, "%d");
}


void CommonViews::ResetButton(std::string id, std::shared_ptr<Parameter> param) {
  ImGui::PushFont(FontService::getService()->icon);
  ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 0.0f));
  if (ImGui::Button(formatString(" ##%s_reset_button", id.c_str()).c_str(), ImVec2(16.0, 16.0))) {
    param->resetValue();
  }
  ImGui::PopFont();
  ImGui::PopStyleVar();
}

void CommonViews::OscillateButton(std::string id, std::shared_ptr<Oscillator> o, std::shared_ptr<Parameter> p) {
  ImGui::PushFont(FontService::getService()->icon);
  ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 0.0f));
  
  std::string buttonTitle;
  if (o->enabled->boolValue) {
    std::string rawTitle = "" "##%s_osc_button";
    buttonTitle = formatString(rawTitle, id.c_str()).c_str();
  } else {
    std::string rawTitle = "" "##%s_osc_button";
    buttonTitle = formatString(rawTitle, id.c_str()).c_str();
  }
  
  if (ImGui::Button(buttonTitle.c_str(), ImVec2(16.0, 16.0))) {
    o->enabled->boolValue = !o->enabled->boolValue;
    OscillationService::getService()->selectOscillator(o, p);
  }
  ImGui::PopStyleVar();
  ImGui::PopFont();
}


void CommonViews::MidiSelector(std::shared_ptr<Parameter> videoParam) {
  bool hasPairing = MidiService::getService()->hasPairingForParameterId(videoParam->paramId);
  bool enabled = false;
  
  
  std::function<void()> learnParamBlock = [videoParam] {
    MidiService::getService()->beginLearning(videoParam);
  };
  
  std::function<void()> stopLearningBlock = [videoParam] {
    // Currently learning the passed Param, so stop Learning
    MidiService::getService()->stopLearning();
  };
  
  std::string buttonTitle;
  std::function<void()> buttonAction;
  
  if (hasPairing) {
    // Already have a pairing, so reset it and start Learning
    buttonTitle = MidiService::getService()->pairingForParameterId(videoParam->paramId) ->descriptor;
    buttonAction = learnParamBlock;
  } else if (MidiService::getService()->isLearning() && MidiService::getService()->learningParam == videoParam) {
    // Currently learning the passed Param, so stop Learning
    buttonTitle = "Learning";
    buttonAction = stopLearningBlock;
  } else if (MidiService::getService()->isLearning()) {
    // Currently learning a different Param, so start Learning this one
    buttonTitle = "Learning Other Parameter";
    buttonAction = learnParamBlock;
  } else {
    // Learn Assignment
    buttonTitle = "Learn Assignment";
    buttonAction = learnParamBlock;
  }
  
  auto idTitle = formatString("%s##%s", buttonTitle.c_str(), videoParam->name.c_str());
  
  if (ImGui::Button(idTitle.c_str())) {
    buttonAction();
  }
  
}

void CommonViews::HorizontallyAligned(float width, float alignment) {
  ImGuiStyle& style = ImGui::GetStyle();
  float avail = ImGui::GetContentRegionAvail().x;
  float off = (avail - width) * alignment;
  if (off > 0.0f)
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + off);
}

void CommonViews::CenteredVerticalLine() {
  HorizontallyAligned(1);
  ImGui::Text("|");
}
