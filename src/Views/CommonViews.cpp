//
//  CommonViews.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 2022-05-12.
//

#include "CommonViews.hpp"
#include "FontService.hpp"
#include "OscillatorView.hpp"
#include "ModulationService.hpp"
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

void CommonViews::ModulationSelector(Parameter *videoParam) {
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

void CommonViews::SliderWithOscillator(std::string title, std::string id, Parameter *param, Oscillator *o) {
  ImGui::Text("%s", title.c_str());
  ImGui::SameLine(0, 20);
  ImGui::SetNextItemWidth(200.0);
  ImGui::SliderFloat(id.c_str(), &param->value, param->min, param->max, "%.3f");
  ImGui::SameLine(0, 20);
  auto checkBox = formatString("Oscillate %s_checkbox", id.c_str());
  ImGui::Checkbox(checkBox.c_str(), &o->enabled);
  if (ImGui::Button(formatString("X %s_reset", id.c_str()).c_str())) {
    param->resetValue();
  }
  OscillatorView::draw(title.c_str(), param, o);
}

void CommonViews::IntSliderWithOscillator(std::string title, std::string id, Parameter *param, Oscillator *o) {
  ImGui::Text("%s", title.c_str());
  ImGui::SameLine(0, 20);
  ImGui::SetNextItemWidth(200.0);
  ImGui::SliderInt(id.c_str(), &param->intValue, param->min, param->max, "%d");
  ImGui::SameLine(0, 20);
  auto checkBox = formatString("Oscillate %s_checkbox", id.c_str());
  ImGui::Checkbox(checkBox.c_str(), &o->enabled);
  if (ImGui::Button(formatString("X %s_reset", id.c_str()).c_str())) {
    param->resetValue();
  }
  OscillatorView::draw(title.c_str(), param, o);
}


void CommonViews::SliderWithInvertOscillator(std::string title, std::string id, Parameter *param, bool *invert, Oscillator *o) {
  ImGui::Text("%s", title.c_str());
  ImGui::SameLine(0, 20);
  ImGui::SetNextItemWidth(150.0);
  ImGui::SliderFloat(id.c_str(), &param->value, param->min, param->max, "%.3f");
  ImGui::SameLine(0, 20);
  auto invertTitle = formatString("Invert ##invert_%s", id.c_str());
  ImGui::Checkbox(invertTitle.c_str(), invert);
  ImGui::SameLine(0, 20);
  auto checkBox = formatString("Oscillate %s_checkbox", id.c_str());
  if (ImGui::Button(formatString("X %s_reset", id.c_str()).c_str())) {
    param->resetValue();
  }
  ImGui::Checkbox(checkBox.c_str(), &o->enabled);
  OscillatorView::draw(title.c_str(), param, o);
}

void CommonViews::MidiSelector(Parameter *videoParam) {
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
