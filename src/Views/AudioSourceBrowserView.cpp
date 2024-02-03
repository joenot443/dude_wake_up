//
//  AudioSourceBrowserView.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 12/23/22.
//

#include "AudioSourceBrowserView.hpp"
#include "CommonViews.hpp"
#include "Colors.hpp"
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

void AudioSourceBrowserView::update() {
  tapper.update();
  AudioSourceService::getService()->selectedAudioSource->audioAnalysis.updateBeat(tapper.beatPerc());
}

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
      // Loudness
      ImGui::TableNextColumn();
      ImGui::Text("Loudness");
      ImGui::SameLine();
      if (ImGui::BeginPopupModal("##Loudness", nullptr, ImGuiPopupFlags_MouseButtonLeft)) {
        MarkdownView("Loudness").draw();
        ImGui::EndPopup();
      }
      if (CommonViews::IconButton(ICON_MD_INFO, "Loudness")) {
        ImGui::OpenPopup("##Loudness");
      }
      ImGui::Text("%s", formatString("%.5F", source->audioAnalysis.rms->value).c_str());
      OscillatorParam original = OscillatorParam (std::static_pointer_cast<Oscillator>(source->audioAnalysis.rmsOscillator), source->audioAnalysis.rms);
      OscillatorParam pulse = OscillatorParam(source->audioAnalysis.rmsAnalysisParam.pulseOscillator, source->audioAnalysis.rmsAnalysisParam.pulse);
      OscillatorParam threshold = OscillatorParam( source->audioAnalysis.rmsAnalysisParam.thresholdOscillator, source->audioAnalysis.rmsAnalysisParam.pulseThreshold);
      std::vector<OscillatorParam> subjects = std::vector<OscillatorParam>(1, original);
      subjects.push_back(pulse);
      subjects.push_back(threshold);
      
      OscillatorView::draw(subjects);
      CommonViews::Slider("Threshold", "##threshold", source->audioAnalysis.rmsAnalysisParam.pulseThreshold);
      CommonViews::Slider("Length", "##length", source->audioAnalysis.rmsAnalysisParam.pulseLength);
      
      ImGui::TableNextColumn();
      
      // Clarity
      ImGui::Text("Clarity");
      ImGui::SameLine();
      if (ImGui::BeginPopupModal("##Clarity", nullptr, ImGuiPopupFlags_MouseButtonLeft)) {
        MarkdownView("Clarity").draw();
        ImGui::EndPopup();
      }
      if (CommonViews::IconButton(ICON_MD_INFO, "Clarity")) {
        ImGui::OpenPopup("##Clarity");
      }
      ImGui::Text("%s", formatString("%.5F", source->audioAnalysis.csd->value).c_str());
      OscillatorView::draw(std::static_pointer_cast<Oscillator>(
                                                                source->audioAnalysis.csdOscillator),
                           source->audioAnalysis.csd);
      
      
      ImGui::TableNextColumn();
      
      // Beats
      ImGui::Text("BPM Tracker");
      ImGui::SameLine();
      if (ImGui::BeginPopupModal("##Energy", nullptr, ImGuiPopupFlags_MouseButtonLeft)) {
        MarkdownView("BPM Tracker").draw();
        ImGui::EndPopup();
      }
      if (CommonViews::IconButton(ICON_MD_INFO, "BPM Tracker")) {
        ImGui::OpenPopup("##BPM Tracker");
      }
      
      
      ImGui::Text("%s", formatString("BPM: %.0f", tapper.bpm()).c_str());
      
      static float lastTapTime = ofGetCurrentTime().seconds;
      if (ImGui::Button("Tap for BPM")) {
        if (ofGetCurrentTime().seconds - lastTapTime > 5.0f) {
          tapper.startFresh();
        } else {
          tapper.tap();
        }
        lastTapTime = ofGetCurrentTime().seconds;
      }
      
      ImGui::SameLine();
      
      if (ImGui::Button("Beat Start")) {
        float bpm = tapper.bpm();
        tapper.startFresh();
        tapper.setBpm(bpm);
      }
      
      ImGui::SameLine();
      
      if (ImGui::Button("Reset")) {
        tapper.startFresh();
      }
      
      ImGui::SameLine();
      const char* buttonText = source->audioAnalysis.bpmEnabled ? "Stop" : "Start";
      if (ImGui::Button(buttonText)) {
        source->audioAnalysis.bpmEnabled = !source->audioAnalysis.bpmEnabled;
      }
      
      if (CommonViews::Slider("BPM", "##bpm", source->audioAnalysis.bpm)) {
        tapper.setBpm(source->audioAnalysis.bpm->value);
      } else {
        source->audioAnalysis.bpm->value = tapper.bpm();
      }
      
      // Only draw graph if we're enabled
      if (source->audioAnalysis.bpmEnabled) {
        source->audioAnalysis.beatPulseOscillator->enabled = true;
        OscillatorView::draw(dynamic_pointer_cast<Oscillator>( source->audioAnalysis.beatPulseOscillator), source->audioAnalysis.beatPulse);
      }
      
      ImGui::TableNextColumn();
      
      // Frequency
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

void AudioSourceBrowserView::keyReleased(int key) {
  tapper.tap();
}

