//
//  AudioSourceBrowserView.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 12/23/22.
//

#include "AudioSourceBrowserView.hpp"
#include "AudioSourceService.hpp"
#include "FontService.hpp"
#include "OscillatorView.hpp"
#include "BarPlotView.hpp"
#include "Strings.hpp"
#include "ofxImGui.h"

void AudioSourceBrowserView::setup() {}

void AudioSourceBrowserView::update() {}

void AudioSourceBrowserView::draw() {
  drawAudioSourceSelector();
  drawSelectedAudioSource();
}

// Draw a list of audio sources as items in a ListBox
void AudioSourceBrowserView::drawAudioSourceSelector() {
  auto sources = AudioSourceService::getService()->audioSources();
  ImGui::PushFont(FontService::getService()->h3);
  ImGui::Text("Audio Source");
  ImGui::PopFont();
  
  if (ImGui::BeginListBox("##audiosources")) {
    for (int i = 0; i < sources.size(); i++) {
      if (ImGui::Selectable(sources[i]->name.c_str())) {
        AudioSourceService::getService()->selectAudioSource(sources[i]);
      }
    }
    ImGui::EndListBox();
  }
}

void AudioSourceBrowserView::drawSelectedAudioSource() {
  auto source = AudioSourceService::getService()->selectedAudioSource;
  if (source) {
    ImGui::PushFont(FontService::getService()->h3);
    ImGui::Text("Selected Audio Source");
    ImGui::PopFont();
    ImGui::Text(source->name.c_str());
    
    if (ImGui::BeginTable("##audioAnalysis", 3)) {
      ImGui::TableNextColumn();
      ImGui::Text("ZCR");
      ImGui::Text(formatString("%.5F", source->audioAnalysis.zcr->value).c_str());
      OscillatorView::draw(std::static_pointer_cast<Oscillator>(
                                                                source->audioAnalysis.zcrOscillator),
                           source->audioAnalysis.zcr);
      
      ImGui::TableNextColumn();
      ImGui::Text("CSD");
      ImGui::Text(formatString("%.5F", source->audioAnalysis.csd->value).c_str());
      OscillatorView::draw(std::static_pointer_cast<Oscillator>(
                                                                source->audioAnalysis.csdOscillator),
                           source->audioAnalysis.csd);
      ImGui::TableNextColumn();
      ImGui::Text("Energy");
      ImGui::Text(
                  formatString("%.5F", source->audioAnalysis.energy->value).c_str());
      OscillatorView::draw(std::static_pointer_cast<Oscillator>(
                                                                source->audioAnalysis.energyOscillator),
                           source->audioAnalysis.energy);
      ImGui::EndTable();
    }
    //    BarPlotView::draw(source->audioAnalysis.melFrequencySpectrum, "mel");
    //    BarPlotView::draw(source->audioAnalysis.magnitudeSpectrum, "mag");
    
  }
}
