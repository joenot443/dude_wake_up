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
#include "AudioSettings.hpp"
#include "ofxImGui.h"

static const float AudioOscillatorHeight = 140.0;

void AudioSourceBrowserView::setup() {}

void AudioSourceBrowserView::update() {
  AudioSourceService::getService()->tapper.update();
  updatePlaybackPosition();
}

void AudioSourceBrowserView::draw() {
  auto source = AudioSourceService::getService()->selectedAudioSource;
  ImVec2 size = ImVec2(ImGui::GetContentRegionAvail().x, LayoutStateService::getService()->audioSettingsViewHeight());
  ImGui::BeginChild("##AudioSourceBrowser", size, ImGuiChildFlags_None, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
  drawStartAnalysisButton();
  ImGui::SameLine();
  drawAudioSourceSelector();
//  ImGui::Dummy(ImVec2(0.0, 0.0));
  if (source->active) {
    drawSampleTrack();
    drawSelectedAudioSource();
  }
  ImGui::EndChild();
}

void AudioSourceBrowserView::drawSampleTrack() {
  if (AudioSourceService::getService()->selectedAudioSource->type() != AudioSourceType_File) return;
  std::shared_ptr<FileAudioSource> source = std::dynamic_pointer_cast<FileAudioSource>( AudioSourceService::getService()->selectedAudioSource);
  std::vector<std::string> options;
  for (auto track : AudioSourceService::getService()->sampleTracks) {
    options.push_back(track->name);
  }
  ImGui::SameLine();
  ImGui::PushItemWidth(200.0);
  if (CommonViews::ShaderOption(AudioSourceService::getService()->selectedSampleTrackParam, options, false)) {
    AudioSourceService::getService()->affirmSampleAudioTrack();
  }
  ImGui::PopItemWidth();
  ImGui::SameLine();
  if (CommonViews::PlaybackSlider(samplePlaybackSliderPosition, source->getTotalDuration())) {
    source->setPlaybackPosition(samplePlaybackSliderPosition->value);
  }
  ImGui::SameLine();
  if (CommonViews::PlayPauseButton("##playPause", !source->isPaused, ImVec2(25.0, 25.0))) {
    if (source->isPaused) {
      source->resumePlayback();
    } else {
      source->pausePlayback();
    }
  }
  ImGui::SameLine();
  ImGui::PushItemWidth(200.0);
  if (ImGui::SliderFloat("Volume", &source->volume, 0.0, 1.0, "%.3f")) {
    source->setVolume(source->volume);
  }
  ImGui::PopItemWidth();
  ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 10.0);
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
  ImGui::SameLine();
  ImGui::SetNextItemWidth(300.0f);
  ImGui::PushFont(FontService::getService()->current->h4);
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
  auto buttonText = source->active ? "Stop Audio Analysis" : "Start Audio Analysis";
  if (ImGui::Button(buttonText)) {
    source->toggle();
    LayoutStateService::getService()->showAudioSettings = !LayoutStateService::getService()->showAudioSettings;
  }
}

void AudioSourceBrowserView::drawSelectedAudioSource() {
  auto source = AudioSourceService::getService()->selectedAudioSource;
  // We are inside "##AudioSourceBrowser".
  
  if (source && source->active) {
    // Push the desired background color for the upcoming child window
    ImGui::PushStyleColor(ImGuiCol_ChildBg, Colors::InnerChildBackgroundColor.Value);
    // Push zero window padding for this child, as we're doing it manually inside
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    
    // Begin a child window specifically to hold the table and its manual padding, applying the background color
    if (ImGui::BeginChild("##audioAnalysisPaddedArea", ImVec2(0, 0), ImGuiChildFlags_None, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse)) { // Auto-size
//      ImGui::Dummy(ImVec2(0.0, 2.0));
      // Add horizontal space before the table
//      ImGui::Indent(8.0f);

      // The table starts here, now manually spaced inside the colored child.
      if (ImGui::BeginTable("##audioAnalysis", 3, ImGuiTableFlags_PadOuterX)) {
        // Loudness
        ImGui::TableNextColumn();
        ImVec2 audioGraphSize = ImVec2((ImGui::GetContentRegionAvail().x - 20.0), AudioOscillatorHeight);
        ImVec2 bpmGraphSize = ImVec2((ImGui::GetContentRegionAvail().x - 20.0), AudioOscillatorHeight - 20.0);
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
        
        OscillatorView::draw(subjects, audioGraphSize, false);
        
        ImGui::TableNextColumn();
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(8.0, 2.0));

        // Beats
        bool isSampleTrack = source->type() == AudioSourceType_File;
        ImGui::Text("BPM");
        ImGui::SameLine();
        if (CommonViews::IconButton(ICON_MD_INFO, "BPM")) {
          ImGui::OpenPopup("##BPM");
        }
        
        // BPM Mode Dropdown
        if (!isSampleTrack) {
          const char* bpmModeItems[] = { "Auto", "Manual", "Link" };
          int currentBpmMode = static_cast<int>(source->audioAnalysis.bpmMode);
          ImGui::SameLine();
          ImGui::SetNextItemWidth(80.0f);
          if (ImGui::Combo("##BpmMode", &currentBpmMode, bpmModeItems, IM_ARRAYSIZE(bpmModeItems))) {
            source->audioAnalysis.bpmMode = static_cast<BpmMode>(currentBpmMode);
            // Reset auto BPM detector when switching to Auto mode
//            if (source->audioAnalysis.bpmMode == BpmMode_Auto) {
//              source->btrackDetector.resetBeatTracking();
//            }
            // Enable Ableton Link when switching to Link mode
            if (source->audioAnalysis.bpmMode == BpmMode_Link) {
              LayoutStateService::getService()->abletonLinkEnabled = true;
              AudioSourceService::getService()->setupAbleton();
            }
          }
        }
        // Set oscillator enabled state and draw graph
        source->audioAnalysis.beatPulseOscillator->enabled = source->audioAnalysis.bpmEnabled || isSampleTrack;
        OscillatorView::draw(std::dynamic_pointer_cast<Oscillator>(source->audioAnalysis.beatPulseOscillator), source->audioAnalysis.beatPulse, audioGraphSize, false);
        
        if (!isSampleTrack) {
          // Show different controls based on BPM mode
          switch (source->audioAnalysis.bpmMode) {
            case BpmMode_Auto: {
              // Show current auto-detected BPM (read-only)
              ImGui::Text("Auto BPM: %.1f", source->audioAnalysis.bpm->value);
              ImGui::SameLine();
              if (CommonViews::PlayPauseButton("##bpmPlayPause", source->audioAnalysis.bpmEnabled, ImVec2(20., 20.0), ImVec2(5.0, 5.0))) {
                source->audioAnalysis.bpmEnabled = !source->audioAnalysis.bpmEnabled;
              }
              break;
            }
            case BpmMode_Manual: {
              // Show manual BPM control with nudge buttons
              if (CommonViews::Slider("BPM", "##bpm", source->audioAnalysis.bpm, ImGui::GetContentRegionAvail().x - 230.0)) {
                AudioSourceService::getService()->tapper.setBpm(source->audioAnalysis.bpm->value);
              }
              ImGui::SameLine();
              
              // BPM nudge buttons
              if (ImGui::Button("-##bpmDown", ImVec2(20, 20))) {
                source->audioAnalysis.bpm->value = std::max(source->audioAnalysis.bpm->min, source->audioAnalysis.bpm->value - 1.0f);
                AudioSourceService::getService()->tapper.setBpm(source->audioAnalysis.bpm->value);
              }
              ImGui::SameLine(0, 2);
              if (ImGui::Button("+##bpmUp", ImVec2(20, 20))) {
                source->audioAnalysis.bpm->value = std::min(source->audioAnalysis.bpm->max, source->audioAnalysis.bpm->value + 1.0f);
                AudioSourceService::getService()->tapper.setBpm(source->audioAnalysis.bpm->value);
              }
              ImGui::SameLine();
              
              static float lastTapTime = ofGetCurrentTime().seconds;
              if (ImGui::Button("Tap")) {
                if (ofGetCurrentTime().seconds - lastTapTime > 5.0f) {
                  AudioSourceService::getService()->tapper.startFresh();
                } else {
                  AudioSourceService::getService()->tapper.tap();
                }
                lastTapTime = ofGetCurrentTime().seconds;
              }
              ImGui::SameLine();
              if (CommonViews::PlayPauseButton("##bpmPlayPause", source->audioAnalysis.bpmEnabled, ImVec2(20., 20.0), ImVec2(5.0, 5.0))) {
                source->audioAnalysis.bpmEnabled = !source->audioAnalysis.bpmEnabled;
              }
              break;
            }
            case BpmMode_Link: {
              // Show current Link BPM (read-only, controlled by Link)
              if (LayoutStateService::getService()->abletonLinkEnabled && AudioSourceService::getService()->link != nullptr) {
                ImGui::Text("Link BPM: %.1f", AudioSourceService::getService()->link->captureAppSessionState().tempo());
              } else {
                ImGui::Text("Link BPM: Not Connected");
              }
              break;
            }
          }
          if (ImGui::BeginPopupModal("##BPM", nullptr, ImGuiPopupFlags_MouseButtonLeft)) {
            MarkdownView("BPM").draw();
            ImGui::EndPopup();
          }
        }
        ImGui::PopStyleVar();
        
        ImGui::TableNextColumn();

        ImGui::Text("Frequency");
        ImGui::SameLine();

        // Frequency view selector with borders and selection colors
        ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.5f, 0.5f, 0.5f, 0.5f));
        ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.3f, 0.5f, 0.8f, 0.8f));
        ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.4f, 0.6f, 0.9f, 0.9f));
        ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0.2f, 0.4f, 0.7f, 1.0f));

        if (ImGui::Selectable("Bands", frequencyViewMode == FrequencyViewMode_Bands, 0, ImVec2(50, 0))) {
          frequencyViewMode = FrequencyViewMode_Bands;
        }
        ImGui::SameLine();
        if (ImGui::Selectable("Bars", frequencyViewMode == FrequencyViewMode_Bars, 0, ImVec2(50, 0))) {
          frequencyViewMode = FrequencyViewMode_Bars;
        }

        ImGui::PopStyleColor(4);
        ImGui::PopStyleVar();
        ImGui::SameLine();

        if (frequencyViewMode == FrequencyViewMode_Bands) {
          // Highs / Mids / Lows
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

          OscillatorView::draw({low, mids, highs}, audioGraphSize, false);
        } else {
          // Frequency Bars
          ImGui::SameLine();
          if (ImGui::BeginPopupModal("##Frequency", nullptr, ImGuiPopupFlags_MouseButtonLeft)) {
            MarkdownView("Frequency").draw();
            ImGui::EndPopup();
          }
          if (CommonViews::IconButton(ICON_MD_INFO, "Frequency")) {
            ImGui::OpenPopup("##Frequency");
          }

          ImVec2 barsGraphSize = ImVec2(audioGraphSize.x - 100.0, audioGraphSize.y);
          BarPlotView::draw(source->audioAnalysis.smoothMelSpectrum, "mel", barsGraphSize);
          ImGui::SameLine();
          ImGui::BeginChild("##FrequencyMods");
          ImGui::Text("Release");
          CommonViews::MiniSlider(source->audioAnalysis.frequencyRelease, false);
          ImGui::Text("Scale");
          CommonViews::MiniSlider(source->audioAnalysis.frequencyScale, false);
          ImGui::EndChild();
        }

        ImGui::EndTable();
      }
    }
    ImGui::EndChild(); // End ##audioAnalysisPaddedArea
    // Pop the zero window padding
    ImGui::PopStyleVar();
    // Pop the background color
    ImGui::PopStyleColor();
  }
}

void AudioSourceBrowserView::keyReleased(int key) {
  AudioSourceService::getService()->tapper.tap();
}

