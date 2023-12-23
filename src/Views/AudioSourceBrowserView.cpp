//
//  AudioSourceBrowserView.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 12/23/22.
//

#include "AudioSourceBrowserView.hpp"
#include "CommonViews.hpp"
#include "MarkdownView.hpp"
#include "Fonts.hpp"
#include "AudioSourceService.hpp"
#include "FontService.hpp"
#include "OscillatorView.hpp"
#include "BarPlotView.hpp"
#include "LayoutStateService.hpp"
#include "Strings.hpp"
#include "ofxImGui.h"

void AudioSourceBrowserView::setup() {}

void AudioSourceBrowserView::update() {}

void AudioSourceBrowserView::draw() {
  auto source = AudioSourceService::getService()->selectedAudioSource;
  ImVec2 size = ImVec2(ImGui::GetContentRegionAvail().x, ofGetWindowHeight() * 2./5.);
  ImGui::SetNextItemOpen(LayoutStateService::getService()->showAudioSettings);
  
  auto title = source->active ? formatString("Audio Reactivity - \t RMS: %0.2f \t CSD: %0.2f \t Energy: %0.2f",
                            source->audioAnalysis.rms->value,
                            source->audioAnalysis.csd->value,
                            source->audioAnalysis.energy->value) : "Audio Reactivity - Disabled";
  
  if (ImGui::CollapsingHeader(title.c_str(), ImGuiTreeNodeFlags_CollapsingHeader))
  {
    LayoutStateService::getService()->showAudioSettings = true;
    ImGui::BeginChild("##AudioSourceBrowser");
    drawSelectedAudioSource();
    ImGui::EndChild();
  } else {
    LayoutStateService::getService()->showAudioSettings = false;
  }
}

// Draw a list of audio sources as items in a ListBox
void AudioSourceBrowserView::drawAudioSourceSelector() {
  auto sources = AudioSourceService::getService()->audioSources();
  ImGui::PushFont(FontService::getService()->h3);
  ImGui::Text("Audio Source");
  ImGui::PopFont();
  
  std::vector<std::string> names = {};
  //allocate space for pointers
  char** out = new char*[sources.size()];
  static int selection = -1;

  for(int i = 0; i < sources.size(); ++i){
    string s = sources[i]->name;
    if (s == AudioSourceService::getService()->selectedAudioSource->name) {
      selection = i;
    }
    //allocate space for c string (note +1 for extra space)
    char* s_cstr = new char[s.size()+1];
    //copy data to prevent mutation of original vector
    strcpy(s_cstr, s.c_str());
    //put pointer in the output
    out[i] = s_cstr;
  }
  ImGui::SetNextItemWidth(200.0);
  if (ImGui::Combo(AudioSourceService::getService()->selectedAudioSource->name.c_str(), &selection, out, sources.size())) {
    AudioSourceService::getService()->selectAudioSource(sources[selection]);
  }
}

void AudioSourceBrowserView::drawSelectedAudioSource() {
  auto source = AudioSourceService::getService()->selectedAudioSource;
  drawAudioSourceSelector();
  
  CommonViews::H3Title("Audio");
  
  if (source) {
    if (ImGui::BeginTable("##audioAnalysis", 4)) {
      ImGui::TableNextColumn();
      ImGui::Text("RMS");
      ImGui::SameLine();
      if (ImGui::BeginPopupModal("##RMS", nullptr, ImGuiPopupFlags_MouseButtonLeft)) {
        MarkdownView("RMS").draw();
        ImGui::EndPopup();
      }
      if (CommonViews::IconButton(ICON_MD_INFO, "RMS")) {
        ImGui::OpenPopup("##RMS");
      }
      ImGui::Text("%s", formatString("%.5F", source->audioAnalysis.rms->value).c_str());
      OscillatorView::draw(std::static_pointer_cast<Oscillator>(
                                                                source->audioAnalysis.rmsOscillator),
                           source->audioAnalysis.rms);
      
      ImGui::TableNextColumn();
      ImGui::Text("CSD");
      ImGui::SameLine();
      if (ImGui::BeginPopupModal("##CSD", nullptr, ImGuiPopupFlags_MouseButtonLeft)) {
        MarkdownView("CSD").draw();
        ImGui::EndPopup();
      }
      if (CommonViews::IconButton(ICON_MD_INFO, "CSD")) {
        ImGui::OpenPopup("##CSD");
      }
      ImGui::Text("%s", formatString("%.5F", source->audioAnalysis.csd->value).c_str());
      OscillatorView::draw(std::static_pointer_cast<Oscillator>(
                                                                source->audioAnalysis.csdOscillator),
                           source->audioAnalysis.csd);
      
      
      ImGui::TableNextColumn();
      ImGui::Text("Energy");
      ImGui::SameLine();
      if (ImGui::BeginPopupModal("##Energy", nullptr, ImGuiPopupFlags_MouseButtonLeft)) {
        MarkdownView("Energy").draw();
        ImGui::EndPopup();
      }
      if (CommonViews::IconButton(ICON_MD_INFO, "Energy")) {
        ImGui::OpenPopup("##Energy");
      }
      ImGui::Text(
                  "%s", formatString("%.5F", source->audioAnalysis.energy->value).c_str());
      OscillatorView::draw(std::static_pointer_cast<Oscillator>(
                                                                source->audioAnalysis.energyOscillator),
                           source->audioAnalysis.energy);
      
      ImGui::TableNextColumn();
      ImGui::Text("Frequency");
      ImGui::SameLine();
      if (ImGui::BeginPopupModal("##Frequency", nullptr, ImGuiPopupFlags_MouseButtonLeft)) {
        MarkdownView("Frequency").draw();
        ImGui::EndPopup();
      }
      if (CommonViews::IconButton(ICON_MD_INFO, "Frequency")) {
        ImGui::OpenPopup("##Frequency");
      }
      BarPlotView::draw(source->audioAnalysis.melFrequencySpectrum, "mel");
      
      ImGui::EndTable();
    }
    ImGui::Columns(2);
    auto buttonText = source->active ? "Stop Analysis" : "Start Analysis";
    
    if (ImGui::Button(buttonText)) {
      source->toggle();
    }
    ImGui::NextColumn();
    ImGui::Text("Selected Audio Source: %s", source->name.c_str());
  }
}
