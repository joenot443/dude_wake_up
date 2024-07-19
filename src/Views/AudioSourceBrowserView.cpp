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
}

void AudioSourceBrowserView::draw() {
  auto source = AudioSourceService::getService()->selectedAudioSource;
  ImVec2 size = ImVec2(ImGui::GetContentRegionAvail().x, source->active ? 200.0 : 20.0);
  ImGui::SetNextItemOpen(LayoutStateService::getService()->showAudioSettings);
  ImGui::SetNextWindowSize(size);
  auto title = source->active ? formatString("Audio Reactivity - \t RMS: %0.2f", source->audioAnalysis.rms->value) : "Audio Reactivity - Disabled";
  if (ImGui::CollapsingHeader(title.c_str(), ImGuiTreeNodeFlags_CollapsingHeader))
  {
    LayoutStateService::getService()->showAudioSettings = true;
    ImGui::BeginChild("##AudioSourceBrowser");
    CommonViews::sSpacing();
    drawStartAnalysisButton();
    ImGui::SameLine();
    drawAudioSourceSelector();
    drawSelectedAudioSource();
    ImGui::EndChild();
  } else {
    LayoutStateService::getService()->showAudioSettings = false;
  }
}

// Draw a list of audio sources as items in a ListBox
void AudioSourceBrowserView::drawAudioSourceSelector() {
  auto sources = AudioSourceService::getService()->audioSources();
  
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
  ImGui::PushFont(FontService::getService()->h4);
  if (ImGui::Combo(AudioSourceService::getService()->selectedAudioSource->name.c_str(), &selection, out, sources.size())) {
    AudioSourceService::getService()->selectAudioSource(sources[selection]);
  }
  ImGui::PopFont();
}

void AudioSourceBrowserView::drawStartAnalysisButton() {
  auto source = AudioSourceService::getService()->selectedAudioSource;
  ImGui::SameLine();
  auto buttonText = source->active ? "Stop Analysis" : "Start Analysis";
  if (ImGui::Button(buttonText)) {
    source->toggle();
  }
}

void AudioSourceBrowserView::drawSelectedAudioSource() {
  auto source = AudioSourceService::getService()->selectedAudioSource;
  if (source && source->active) {
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
      ImGui::SameLine();
      CommonViews::ShaderCheckbox(source->audioAnalysis.enableRmsPulse, true);
      OscillatorParam original = OscillatorParam (std::static_pointer_cast<Oscillator>(source->audioAnalysis.rmsOscillator), source->audioAnalysis.rms);
      std::vector<OscillatorParam> subjects = {original};
      
      // Add the pulser if enabled
      if (source->audioAnalysis.enableRmsPulse->boolValue) {
        OscillatorParam pulse = OscillatorParam(source->audioAnalysis.rmsAnalysisParam.pulseOscillator, source->audioAnalysis.rmsAnalysisParam.pulse);
        OscillatorParam threshold = OscillatorParam( source->audioAnalysis.rmsAnalysisParam.thresholdOscillator, source->audioAnalysis.rmsAnalysisParam.pulseThreshold);
        subjects.push_back(pulse);
        subjects.push_back(threshold);
      }
      
      OscillatorView::draw(subjects);
      
      if (source->audioAnalysis.enableRmsPulse->boolValue) {
        CommonViews::Slider("Threshold", "##threshold", source->audioAnalysis.rmsAnalysisParam.pulseThreshold);
        CommonViews::Slider("Length", "##length", source->audioAnalysis.rmsAnalysisParam.pulseLength);
      }
      
      ImGui::TableNextColumn();
      
      // Beats
      if (LayoutStateService::getService()->abletonLinkEnabled) {
        ImGui::Text("%s", formatString("Ableton Tempo: %f", AudioSourceService::getService()->link.captureAppSessionState().tempo()).c_str());
      } else {
        ImGui::Text("BPM Tracker");
      }
      ImGui::SameLine();
      if (ImGui::BeginPopupModal("##BPMTrackerInfo", nullptr, ImGuiPopupFlags_MouseButtonLeft)) {
        MarkdownView("BPM Tracker").draw();
        ImGui::EndPopup();
      }
      
      if (CommonViews::IconButton(ICON_MD_INFO, "BPM Tracker")) {
        ImGui::OpenPopup("##BPM Tracker");
      }
      ImGui::SameLine();
      ImGui::Checkbox("Ableton Link?", &LayoutStateService::getService()->abletonLinkEnabled);
      
      // Only draw graph if we're enabled
      if (source->audioAnalysis.bpmEnabled) {
        source->audioAnalysis.beatPulseOscillator->enabled = true;
        OscillatorView::draw(dynamic_pointer_cast<Oscillator>( source->audioAnalysis.beatPulseOscillator), source->audioAnalysis.beatPulse);
      } else {
        ImGui::NewLine();
      }
      
      // Draw BPM controls
      if (!LayoutStateService::getService()->abletonLinkEnabled) {
        ImGui::SameLine();

        if (ImGui::BeginChild("##BPMControls", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionMax().y))) {
          
          static float lastTapTime = ofGetCurrentTime().seconds;
          if (ImGui::Button("Tap for BPM")) {
            if (ofGetCurrentTime().seconds - lastTapTime > 5.0f) {
              tapper.startFresh();
            } else {
              tapper.tap();
            }
            lastTapTime = ofGetCurrentTime().seconds;
          }
          
          
          if (ImGui::Button("Beat Start")) {
            float bpm = tapper.bpm();
            tapper.startFresh();
            tapper.setBpm(bpm);
            source->audioAnalysis.bpmEnabled = true;
          }
                    
          if (ImGui::Button("Reset")) {
            tapper.startFresh();
          }
          
          const char* buttonText = source->audioAnalysis.bpmEnabled ? "Stop" : "Start";
          if (ImGui::Button(buttonText)) {
            source->audioAnalysis.bpmEnabled = !source->audioAnalysis.bpmEnabled;
          }
          
          if (CommonViews::Slider("BPM", "##bpm", source->audioAnalysis.bpm)) {
            tapper.setBpm(source->audioAnalysis.bpm->value);
          } else {
            source->audioAnalysis.bpm->value = tapper.bpm();
          }
        }
        ImGui::EndChild();
      }
      
  
      
      ImGui::TableNextColumn();
      
      // Highs / Mids / Lows
      ImGui::Text("Frequency Bands");
      ImGui::SameLine();
      if (ImGui::BeginPopupModal("##FrequencyBands", nullptr, ImGuiPopupFlags_MouseButtonLeft)) {
        MarkdownView("FrequencyBands").draw();
        ImGui::EndPopup();
      }
      if (CommonViews::IconButton(ICON_MD_INFO, "FrequencyBands")) {
        ImGui::OpenPopup("##FrequencyBands");
      }
      OscillatorParam low = OscillatorParam(source->audioAnalysis.lowsOscillator, source->audioAnalysis.lowsAnalysisParam.param);
      OscillatorParam mids = OscillatorParam(source->audioAnalysis.midsOscillator, source->audioAnalysis.midsAnalysisParam.param);
      OscillatorParam highs = OscillatorParam(source->audioAnalysis.highsOscillator, source->audioAnalysis.highsAnalysisParam.param);
      
      OscillatorView::draw({low, mids, highs});
      
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
      BarPlotView::draw(source->audioAnalysis.smoothMelSpectrum, "mel");
      ImGui::SameLine();
      CommonViews::Slider("Release", "##frequencyRelease", source->audioAnalysis.frequencyRelease);
      ImGui::EndTable();
    }
  }
}

void AudioSourceBrowserView::keyReleased(int key) {
  tapper.tap();
}

