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
#include "FileAudioSource.hpp"
#include "BarPlotView.hpp"
#include "LayoutStateService.hpp"
#include "Strings.hpp"
#include "ofxImGui.h"

void AudioSourceBrowserView::setup() {}

void AudioSourceBrowserView::update() {
  AudioSourceService::getService()->tapper.update();
  updatePlaybackPosition();
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
    drawSampleTrack();
    drawSelectedAudioSource();
    ImGui::EndChild();
  } else {
    LayoutStateService::getService()->showAudioSettings = false;
  }
}

void AudioSourceBrowserView::drawSampleTrack() {
  if (AudioSourceService::getService()->selectedAudioSource->type() != AudioSourceType_File) return;
  std::shared_ptr<FileAudioSource> source = std::dynamic_pointer_cast<FileAudioSource>( AudioSourceService::getService()->selectedAudioSource);
  std::vector<std::string> options;
  for (auto track : AudioSourceService::getService()->sampleTracks) {
    options.push_back(track->name);
  }
  ImGui::SameLine();
  ImGui::PushItemWidth(150.0);
  if (CommonViews::ShaderOption(AudioSourceService::getService()->selectedSampleTrackParam, options, false)) {
    AudioSourceService::getService()->affirmSampleAudioTrack();
  }
  ImGui::PopItemWidth();
  ImGui::SameLine();
  CommonViews::PlaybackSlider(samplePlaybackSliderPosition, source->getTotalDuration());
  auto playPauseIcon = source->isPaused ? ICON_MD_PLAY_ARROW : ICON_MD_PAUSE;
  ImGui::SameLine();
  if (CommonViews::IconButton(playPauseIcon, "Play/Pause")) {
    if (source->isPaused) {
      source->resumePlayback();
    } else {
      source->pausePlayback();
    }
  }
}

void AudioSourceBrowserView::updatePlaybackPosition() {
  if (AudioSourceService::getService()->selectedAudioSource->type() != AudioSourceType_File) return;
  
  std::shared_ptr<FileAudioSource> source = std::dynamic_pointer_cast<FileAudioSource>( AudioSourceService::getService()->selectedAudioSource);
  
  samplePlaybackSliderPosition->value = source->getPlaybackPosition();
  
  if (((float) fabs(samplePlaybackSliderPosition->value - source->getPlaybackPosition()) > 0.001)) {
    source->setPlaybackPosition(samplePlaybackSliderPosition->value);
  }
}

// Draw a list of audio sources as items in a ListBox
void AudioSourceBrowserView::drawAudioSourceSelector() {
  // Keep persistent storage for names and char* pointers
  static std::vector<std::string> names;
  static std::vector<char*> out;
  static int selection = -1;
  
  auto sources = AudioSourceService::getService()->audioSources();
  
  // Resize vectors only when the number of sources changes
  if (names.size() != sources.size()) {
    names.resize(sources.size());
    out.resize(sources.size());
    
    for (int i = 0; i < sources.size(); ++i) {
      names[i] = sources[i]->name;
      
      // Allocate memory for char* only once
      if (out[i] == nullptr) {
        out[i] = new char[names[i].size() + 1];
      }
      
      // Copy the string to the char* buffer
      strcpy(out[i], names[i].c_str());
      
      if (names[i] == AudioSourceService::getService()->selectedAudioSource->name) {
        selection = i;
      }
    }
  } else {
    // Update selection if needed
    for (int i = 0; i < sources.size(); ++i) {
      if (names[i] == AudioSourceService::getService()->selectedAudioSource->name) {
        selection = i;
        break;
      }
    }
  }
  CommonViews::HSpacing(10.0);
  ImGui::SameLine();
  ImGui::SetNextItemWidth(400.0);
  ImGui::PushFont(FontService::getService()->h4);
  ImGui::PushID("##AudioSourceSelector");
  if (ImGui::Combo("", &selection, out.data(), sources.size())) {
    AudioSourceService::getService()->selectAudioSource(sources[selection]);
  }
  ImGui::PopID();
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
      OscillatorParam original = OscillatorParam (std::static_pointer_cast<Oscillator>(source->audioAnalysis.rmsOscillator), source->audioAnalysis.rms);
      std::vector<OscillatorParam> subjects = {original};
      
      OscillatorView::draw(subjects);
      
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
              AudioSourceService::getService()->tapper.startFresh();
            } else {
              AudioSourceService::getService()->tapper.tap();
            }
            lastTapTime = ofGetCurrentTime().seconds;
          }
          
          
          if (ImGui::Button("Beat Start")) {
            float bpm = AudioSourceService::getService()->tapper.bpm();
            AudioSourceService::getService()->tapper.startFresh();
            AudioSourceService::getService()->tapper.setBpm(bpm);
            source->audioAnalysis.bpmEnabled = true;
          }
          
          if (ImGui::Button("Reset")) {
            AudioSourceService::getService()->tapper.startFresh();
          }
          
          const char* buttonText = source->audioAnalysis.bpmEnabled ? "Stop" : "Start";
          if (ImGui::Button(buttonText)) {
            source->audioAnalysis.bpmEnabled = !source->audioAnalysis.bpmEnabled;
          }
          
          if (CommonViews::Slider("BPM", "##bpm", source->audioAnalysis.bpm)) {
            AudioSourceService::getService()->tapper.setBpm(source->audioAnalysis.bpm->value);
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
      ImGui::BeginChild("##FrequencyMods");
      CommonViews::MiniSlider(source->audioAnalysis.frequencyRelease);
      CommonViews::MiniSlider(source->audioAnalysis.frequencyScale);
      ImGui::EndChild();
      ImGui::EndTable();
    }
  }
}

void AudioSourceBrowserView::keyReleased(int key) {
  AudioSourceService::getService()->tapper.tap();
}

