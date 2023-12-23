//
//  CommonViews.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 2022-05-12.
//

#include "CommonViews.hpp"
#include "AudioSourceService.hpp"
#include "FontService.hpp"
#include "Fonts.hpp"
#include "MidiService.hpp"
#include "ModulationService.hpp"
#include "OscillationService.hpp"
#include "ofMain.h"
#include "OscillatorView.hpp"
#include "Strings.hpp"
#include "imgui.h"

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
  sSpacing();
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
  if (ImGui::Button(formatString("Select Param%s", param->name.c_str()).c_str()))
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

void CommonViews::ShaderCheckbox(std::shared_ptr<Parameter> param)
{
  sSpacing();
  ImGui::Checkbox(param->name.c_str(), &param->boolValue);
  sSpacing();
}

void CommonViews::H3Title(std::string title)
{
  CommonViews::Spacing(2);
  ImGui::PushFont(FontService::getService()->h3);
  ImGui::Text("%s", title.c_str());
  CommonViews::Spacing(2);
  ImGui::PopFont();
}

void CommonViews::H4Title(std::string title)
{
  CommonViews::Spacing(2);
  ImGui::PushFont(FontService::getService()->h4);
  ImGui::Text("%s", title.c_str());
  CommonViews::Spacing(2);
  ImGui::PopFont();
}

void CommonViews::Slider(std::string title, std::string id,
                         std::shared_ptr<Parameter> param)
{
  ImGui::SetNextItemWidth(200.0);
  ImGui::SliderFloat(idString(id).c_str(), &param->value, param->min,
                     param->max, "%.3f");
  ImGui::SameLine(0, 20);
  ResetButton(id, param);
}

void CommonViews::ShaderColor(std::shared_ptr<Parameter> param)
{
  ImGui::ColorEdit3(param->name.c_str(), param->color->data());
  ImGui::SameLine(0, 20);
  ResetButton(param->paramId, param);
}

void CommonViews::PlaybackSlider(ofVideoPlayer *player)
{
  // Get the available space and center our slider within it, with 20px of padding on each side.
  auto space = ImGui::GetContentRegionAvail();
  auto pos = ImGui::GetCursorScreenPos();
  ImGui::SetNextItemWidth(space.x - 40.0f);
  ImGui::SetCursorScreenPos(pos + ImVec2(20.0f, 0.0f));
  auto position = player->getPosition();

  if (ImGui::SliderFloat("##playback", &position, 0.0f, 1.0f))
  {
    player->setPosition(position);
  }
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
