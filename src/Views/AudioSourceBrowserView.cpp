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
  ImGui::PushFont(FontService::getService()->current->b);
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
//    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 0.0f));

    // Begin a child window specifically to hold the table and its manual padding, applying the background color
    if (ImGui::BeginChild("##audioAnalysisPaddedArea", ImVec2(0, 0), ImGuiChildFlags_None, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse)) { // Auto-size

      // The table starts here, now manually spaced inside the colored child.
      if (ImGui::BeginTable("##audioAnalysis", 3, ImGuiTableFlags_PadOuterX)) {
        // Loudness
        ImGui::TableNextColumn();
        ImVec2 audioGraphSize = ImVec2((ImGui::GetContentRegionAvail().x - 20.0), AudioOscillatorHeight);
        ImVec2 bpmGraphSize = ImVec2((ImGui::GetContentRegionAvail().x - 20.0), AudioOscillatorHeight - 20.0);
        ImGui::Text("Loudness");
        OscillatorParam original = OscillatorParam (std::static_pointer_cast<Oscillator>(source->audioAnalysis.rmsOscillator), source->audioAnalysis.rms);
        std::vector<OscillatorParam> subjects = {original};

        OscillatorView::draw(subjects, audioGraphSize, false);

        CommonViews::MiniSlider(source->audioAnalysis.rmsAnalysisParam.release, true, ImGuiSliderFlags_Logarithmic);

        ImGui::TableNextColumn();
//        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(8.0, 2.0));

        // Beats
        bool isSampleTrack = source->type() == AudioSourceType_File;
        ImGui::Text("BPM");

        // BPM Lock Button
        ImGui::SameLine();
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() +
                             4.0);
        std::string lockIcon = source->audioAnalysis.bpmLocked->boolValue ? "locked.png" : "unlocked.png";
        if (CommonViews::ImageButton("BPMLock", lockIcon, ImVec2(20.0, 20.0), ImVec2(2.0, 2.0))) {
          source->audioAnalysis.bpmLocked->toggleValue();
        }

        ImGui::SameLine();
        ImGui::Text("%.0f BPM", source->audioAnalysis.bpm->value);

        // Set oscillator enabled state and draw graph
        source->audioAnalysis.beatPulseOscillator->enabled = source->audioAnalysis.bpmEnabled || isSampleTrack;
        OscillatorView::draw(std::dynamic_pointer_cast<Oscillator>(source->audioAnalysis.beatPulseOscillator), source->audioAnalysis.beatPulse, audioGraphSize, false);

        // BPM Mode Selector - underneath the graph
        if (!isSampleTrack) {
          ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2.0, 2.0));
          if (CommonViews::Selector(source->audioAnalysis.bpmModeParam, {"Auto", "Manual", "Link"}, 100.0f)) {
            BpmMode oldMode = source->audioAnalysis.bpmMode;
            source->audioAnalysis.bpmMode = static_cast<BpmMode>(source->audioAnalysis.bpmModeParam->intValue);

            // When switching TO Manual mode, reset bpmStartTime to sync with current BPM
            if (source->audioAnalysis.bpmMode == BpmMode_Manual && oldMode != BpmMode_Manual) {
              auto currentTime = std::chrono::steady_clock::now();
              source->audioAnalysis.bpmStartTime = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime.time_since_epoch()).count();
            }

            // Enable Ableton Link when switching to Link mode
            if (source->audioAnalysis.bpmMode == BpmMode_Link) {
              AudioSourceService::getService()->setupAbleton();
            }
          }
          ImGui::PopStyleVar();
          ImGui::SameLine();
        }

        if (isSampleTrack) {
          // BPM Nudge buttons for sample tracks
          ImGui::Text("Nudge:");
          ImGui::SameLine();
          if (CommonViews::ImageButton("nudgeDownSample", "minus.png", ImVec2(20.0, 20.0), ImVec2(2.0, 2.0))) {
            source->audioAnalysis.bpmNudge->value = std::max(source->audioAnalysis.bpmNudge->min, source->audioAnalysis.bpmNudge->value - 0.1f);
          }
          ImGui::SameLine(0, 2);
          if (CommonViews::ImageButton("nudgeUpSample", "plus.png", ImVec2(20.0, 20.0), ImVec2(2.0, 2.0))) {
            source->audioAnalysis.bpmNudge->value = std::min(source->audioAnalysis.bpmNudge->max, source->audioAnalysis.bpmNudge->value + 0.1f);
          }
          ImGui::SameLine();
          ImGui::Text("%.2f", source->audioAnalysis.bpmNudge->value);
        }

        if (!isSampleTrack) {
          // Show different controls based on BPM mode
          switch (source->audioAnalysis.bpmMode) {
            case BpmMode_Auto: {
              // Show current auto-detected BPM (read-only)
              ImGui::Text("BPM: %.1f", source->audioAnalysis.bpm->value);
              ImGui::SameLine();
              if (CommonViews::PlayPauseButton("##bpmPlayPause", source->audioAnalysis.bpmEnabled, ImVec2(20., 20.0), ImVec2(5.0, 5.0))) {
                source->audioAnalysis.bpmEnabled = !source->audioAnalysis.bpmEnabled;
              }

              // BPM Nudge buttons
              ImGui::Text("Nudge:");
              ImGui::SameLine();
              if (CommonViews::ImageButton("nudgeDownAuto", "minus.png", ImVec2(20.0, 20.0), ImVec2(2.0, 2.0))) {
                source->audioAnalysis.bpmNudge->value = std::max(source->audioAnalysis.bpmNudge->min, source->audioAnalysis.bpmNudge->value - 0.1f);
              }
              ImGui::SameLine(0, 2);
              if (CommonViews::ImageButton("nudgeUpAuto", "plus.png", ImVec2(20.0, 20.0), ImVec2(2.0, 2.0))) {
                source->audioAnalysis.bpmNudge->value = std::min(source->audioAnalysis.bpmNudge->max, source->audioAnalysis.bpmNudge->value + 0.1f);
              }
              ImGui::SameLine();
              ImGui::Text("%.2f", source->audioAnalysis.bpmNudge->value);
              break;
            }
            case BpmMode_Manual: {
              // Show manual BPM control with nudge buttons
              if (CommonViews::Slider("BPM", "##bpm", source->audioAnalysis.bpm, ImGui::GetContentRegionAvail().x - 230.0)) {
                AudioSourceService::getService()->tapper.setBpm(source->audioAnalysis.bpm->value);
              }
              ImGui::SameLine();

              // BPM adjustment buttons
              if (CommonViews::ImageButton("bpmDown", "minus.png", ImVec2(20.0, 20.0), ImVec2(2.0, 2.0))) {
                source->audioAnalysis.bpm->value = std::max(source->audioAnalysis.bpm->min, source->audioAnalysis.bpm->value - 1.0f);
                AudioSourceService::getService()->tapper.setBpm(source->audioAnalysis.bpm->value);
              }
              ImGui::SameLine(0, 2);
              if (CommonViews::ImageButton("bpmUp", "plus.png", ImVec2(20.0, 20.0), ImVec2(2.0, 2.0))) {
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

              // Phase nudge buttons
              ImGui::Text("Nudge:");
              ImGui::SameLine();
              if (CommonViews::ImageButton("nudgeDownManual", "minus.png", ImVec2(20.0, 20.0), ImVec2(2.0, 2.0))) {
                source->audioAnalysis.bpmNudge->value = std::max(source->audioAnalysis.bpmNudge->min, source->audioAnalysis.bpmNudge->value - 0.1f);
              }
              ImGui::SameLine(0, 2);
              if (CommonViews::ImageButton("nudgeUpManual", "plus.png", ImVec2(20.0, 20.0), ImVec2(2.0, 2.0))) {
                source->audioAnalysis.bpmNudge->value = std::min(source->audioAnalysis.bpmNudge->max, source->audioAnalysis.bpmNudge->value + 0.1f);
              }
              ImGui::SameLine();
              ImGui::Text("%.2f", source->audioAnalysis.bpmNudge->value);
              break;
            }
            case BpmMode_Link: {
              // Show current Link BPM (read-only, controlled by Link)
              if (LayoutStateService::getService()->abletonLinkEnabled && AudioSourceService::getService()->link != nullptr) {
                ImGui::Text("Link BPM: %.1f", AudioSourceService::getService()->link->captureAppSessionState().tempo());
              } else {
                ImGui::Text("Link BPM: Not Connected");
              }

              // Phase nudge buttons
              ImGui::Text("Nudge:");
              ImGui::SameLine();
              if (CommonViews::ImageButton("nudgeDownLink", "minus.png", ImVec2(20.0, 20.0), ImVec2(2.0, 2.0))) {
                source->audioAnalysis.bpmNudge->value = std::max(source->audioAnalysis.bpmNudge->min, source->audioAnalysis.bpmNudge->value - 0.1f);
              }
              ImGui::SameLine(0, 2);
              if (CommonViews::ImageButton("nudgeUpLink", "plus.png", ImVec2(20.0, 20.0), ImVec2(2.0, 2.0))) {
                source->audioAnalysis.bpmNudge->value = std::min(source->audioAnalysis.bpmNudge->max, source->audioAnalysis.bpmNudge->value + 0.1f);
              }
              ImGui::SameLine();
              ImGui::Text("%.2f", source->audioAnalysis.bpmNudge->value);
              break;
            }
          }
        }

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
        ImGui::SameLine();
        if (ImGui::Selectable("Waveform", frequencyViewMode == FrequencyViewMode_Waveform, 0, ImVec2(70, 0))) {
          frequencyViewMode = FrequencyViewMode_Waveform;
        }

        ImGui::PopStyleColor(4);
        ImGui::PopStyleVar();

        if (frequencyViewMode == FrequencyViewMode_Bands) {
          // Highs / Mids / Lows
          OscillatorParam low = OscillatorParam(source->audioAnalysis.lowsOscillator, source->audioAnalysis.lowsAnalysisParam.param);
          OscillatorParam mids = OscillatorParam(source->audioAnalysis.midsOscillator, source->audioAnalysis.midsAnalysisParam.param);
          OscillatorParam highs = OscillatorParam(source->audioAnalysis.highsOscillator, source->audioAnalysis.highsAnalysisParam.param);

          OscillatorView::draw({low, mids, highs}, audioGraphSize, false);
        } else if (frequencyViewMode == FrequencyViewMode_Bars) {
          // Frequency Bars
          ImVec2 barsGraphSize = ImVec2(audioGraphSize.x - 100.0, audioGraphSize.y);
          BarPlotView::draw(source->audioAnalysis.smoothMelSpectrum, "mel", barsGraphSize);
          ImGui::SameLine();
          ImGui::BeginChild("##FrequencyMods");

          // Smoothing mode dropdown
          const char* smoothingModeItems[] = { "Exponential", "Mov. Avg", "None", "Peak Hold" };
          int currentSmoothingMode = static_cast<int>(source->audioAnalysis.smoothingMode);
          ImGui::Text("Mode");
          ImGui::SetNextItemWidth(90.0f);
          if (ImGui::Combo("##SmoothingMode", &currentSmoothingMode, smoothingModeItems, IM_ARRAYSIZE(smoothingModeItems))) {
            source->audioAnalysis.smoothingMode = static_cast<SmoothingMode>(currentSmoothingMode);
          }

          ImGui::Text("Release");
          CommonViews::MiniSlider(source->audioAnalysis.frequencyRelease, false, ImGuiSliderFlags_Logarithmic);
          ImGui::Text("Scale");
          CommonViews::MiniSlider(source->audioAnalysis.frequencyScale, false);
          ImGui::EndChild();
        } else if (frequencyViewMode == FrequencyViewMode_Waveform) {
          // Waveform visualization

          ImVec2 waveformGraphSize = ImVec2(audioGraphSize.x - 100.0, audioGraphSize.y);

          // Draw waveform using ImGui plotting
          ImDrawList* draw_list = ImGui::GetWindowDrawList();
          ImVec2 canvas_pos = ImGui::GetCursorScreenPos();
          ImVec2 canvas_size = waveformGraphSize;

          // Background
          draw_list->AddRectFilled(canvas_pos, ImVec2(canvas_pos.x + canvas_size.x, canvas_pos.y + canvas_size.y), IM_COL32(20, 20, 20, 255));

          // Center line
          float center_y = canvas_pos.y + canvas_size.y * 0.5f;
          draw_list->AddLine(ImVec2(canvas_pos.x, center_y), ImVec2(canvas_pos.x + canvas_size.x, center_y), IM_COL32(80, 80, 80, 255), 1.0f);

          // Draw waveform - use waveform data and apply scale directly without clamping for visualization
          // Make a local copy to avoid threading issues
          std::vector<float> waveformCopy = source->audioAnalysis.waveform;

          if (waveformCopy.size() >= 256) {
            float scale = source->audioAnalysis.frequencyScale->value;
            float release = source->audioAnalysis.frequencyRelease->value;

            // Apply exponential smoothing manually for visualization (using release value)
            static std::vector<float> visualSmooth(256, 0.0f);
            if (visualSmooth.size() != 256) visualSmooth.resize(256, 0.0f);

            // First update all smoothed values
            size_t maxIndex = std::min(size_t(256), waveformCopy.size());
            for (size_t i = 0; i < maxIndex; i++) {
              visualSmooth[i] = visualSmooth[i] * release + waveformCopy[i] * (1.0f - release);
            }

            // Then draw lines between consecutive points
            for (size_t i = 0; i < 255; i++) {
              float x1 = canvas_pos.x + (float)i / 256.0f * canvas_size.x;
              float x2 = canvas_pos.x + (float)(i + 1) / 256.0f * canvas_size.x;

              // Map waveform values with scale applied, but don't clamp for visualization
              float y1 = center_y - visualSmooth[i] * scale * canvas_size.y * 0.4f;
              float y2 = center_y - visualSmooth[i + 1] * scale * canvas_size.y * 0.4f;

              draw_list->AddLine(ImVec2(x1, y1), ImVec2(x2, y2), IM_COL32(100, 200, 255, 255), 2.0f);
            }
          }

          ImGui::Dummy(canvas_size);

          ImGui::SameLine();
          ImGui::BeginChild("##WaveformMods");

          // Smoothing mode dropdown
          const char* smoothingModeItems[] = { "Exponential", "Mov. Avg", "None", "Peak Hold" };
          int currentSmoothingMode = static_cast<int>(source->audioAnalysis.smoothingMode);
          ImGui::Text("Mode");
          ImGui::SetNextItemWidth(90.0f);
          if (ImGui::Combo("##SmoothingModeWaveform", &currentSmoothingMode, smoothingModeItems, IM_ARRAYSIZE(smoothingModeItems))) {
            source->audioAnalysis.smoothingMode = static_cast<SmoothingMode>(currentSmoothingMode);
          }

          ImGui::Text("Release");
          CommonViews::MiniSlider(source->audioAnalysis.frequencyRelease, false, ImGuiSliderFlags_Logarithmic);
          ImGui::Text("Scale");
          CommonViews::MiniSlider(source->audioAnalysis.frequencyScale, false);
          ImGui::EndChild();
        }

        ImGui::EndTable();
      }
    }
    ImGui::EndChild(); // End ##audioAnalysisPaddedArea
    // Pop the zero window padding
//    ImGui::PopStyleVar();
    // Pop the background color
    ImGui::PopStyleColor();
  }
}

void AudioSourceBrowserView::keyReleased(int key) {
  AudioSourceService::getService()->tapper.tap();
}

