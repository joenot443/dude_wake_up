//
//  VideoFeedSettingsView.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 2022-05-09.
//

#include "CommonViews.hpp"

#include "VideoSettingsView.hpp"
#include "ofxImGui.h"
#include "Console.hpp"
#include "ConfigService.hpp"
#include "MidiService.hpp"
#include "VideoSettingsView.hpp"
#include "FontService.hpp"

const static ofVec2f windowSize = ofVec2f(800, 600);

void VideoSettingsView::setup() {
  feedback0SettingsView.feedbackSettings = &videoSettings->feedback0Settings;
  feedback1SettingsView.feedbackSettings = &videoSettings->feedback1Settings;
  feedback2SettingsView.feedbackSettings = &videoSettings->feedback2Settings;
  styleWindow();
}

void VideoSettingsView::styleWindow() {
  ImGuiIO& io = ImGui::GetIO();
  ImGuiStyle& style = ImGui::GetStyle();
  
  style.WindowPadding = ImVec2(20, 20);
  style.FramePadding = ImVec2(14, 2);
  style.ItemSpacing = ImVec2(8, 3);
  style.ItemInnerSpacing = ImVec2(2, 4);
  style.WindowRounding = 12.f;
}

void VideoSettingsView::update() {
  
}

void VideoSettingsView::draw() {
  if (!hasDrawn) {
    MidiService::getService()->loadConfigFile();
    hasDrawn = true;
  }

  ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_MenuBar;
  ImGui::SetNextWindowSize(windowSize);
  auto name = std::string("##%d Video Feed Tabs", videoSettings->streamId);
  
  ImGui::Begin(name.c_str(), NULL, windowFlags);
  
  if (ImGui::BeginMenuBar())
  {
      if (ImGui::BeginMenu("File"))
      {
        if (ImGui::MenuItem("Save Config", "CMD+S")) {
          auto t = std::time(nullptr);
          auto tm = *std::localtime(&t);
          stringstream timeStream;
          timeStream << std::put_time(&tm, "%d-%m-%Y %H-%M-%S") << std::endl;
          timeStream << " Config.json";
          ofFileDialogResult result = ofSystemSaveDialog(timeStream.str(), "Save config file");
          if(result.bSuccess) {
            ConfigService::getService()->saveConfigFile(videoSettings, result.getPath());
          }
        }
        if (ImGui::MenuItem("Load Config", "CMD+O")) {
          ofFileDialogResult result = ofSystemLoadDialog("Open config file");
          if (result.getPath().length()) {
              ConfigService::getService()->loadConfigFile(result.getPath());
          }
        }
          ImGui::EndMenu();
      }
    ImGui::EndMenuBar();
  }
  
  ImGui::PushFont(FontService::getService()->h2);
  ImGui::Text("Video Feed Settings - %d", videoSettings->streamId);
  ImGui::PopFont();
  ImGui::Separator();
  
  
  if (ImGui::BeginTabBar(name.c_str(), ImGuiTabBarFlags_None))
  {
    if (ImGui::BeginTabItem("Video Settings"))
    {
      ImGui::Columns(2, "videofeedsettings", false);
      drawHSB();
      drawPixelation();
      ImGui::NextColumn();
      drawBlurSharpen();
      drawTransform();
      ImGui::Columns(1);
      CommonViews::Spacing(16);
      ImGui::EndTabItem();
    }
    if (ImGui::BeginTabItem("Feedback 1"))
    {
      feedback0SettingsView.draw();
      if (ImGui::Button("Clear Feedback")) {
        videoStream->shouldClearFrameBuffer = true;
      }
      ImGui::EndTabItem();
    }
    if (ImGui::BeginTabItem("Feedback 2"))
    {
      feedback1SettingsView.draw();
      ImGui::EndTabItem();
    }
    if (ImGui::BeginTabItem("Feedback 3"))
    {
      feedback2SettingsView.draw();
      ImGui::EndTabItem();
    }
    ImGui::EndTabBar();
  }
  ImGui::Separator();
  if (ImGui::Button("Close Stream")) {
    closeStream(videoSettings->streamId);
  }
  
  ImGui::End();
}

void VideoSettingsView::drawMenu() {
  
}


void VideoSettingsView::drawHSB() {
  CommonViews::H3Title("Basic (HSB)");
  
  // Hue
  CommonViews::SliderWithInvertOscillator("Hue", "##hue", &videoSettings->hsbSettings.hue, &videoSettings->hsbSettings.invertHue, &videoSettings->hsbSettings.hueOscillator);
  CommonViews::ModulationSelector(&videoSettings->hsbSettings.hue);
  CommonViews::MidiSelector(&videoSettings->hsbSettings.hue);
  
  // Saturation
  CommonViews::SliderWithInvertOscillator("Saturation", "##saturation", &videoSettings->hsbSettings.saturation, &videoSettings->hsbSettings.invertSaturation, &videoSettings->hsbSettings.saturationOscillator);
  CommonViews::ModulationSelector(&videoSettings->hsbSettings.saturation);
  CommonViews::MidiSelector(&videoSettings->hsbSettings.saturation);

  // Brightness
  CommonViews::SliderWithInvertOscillator("Brightness", "##brightness", &videoSettings->hsbSettings.brightness, &videoSettings->hsbSettings.invertBrightness, &videoSettings->hsbSettings.brightnessOscillator);
  CommonViews::ModulationSelector(&videoSettings->hsbSettings.brightness);
  CommonViews::MidiSelector(&videoSettings->hsbSettings.brightness);
}

void VideoSettingsView::drawBlurSharpen() {
  CommonViews::H3Title("Blur");
  
  // Amount
  CommonViews::SliderWithOscillator("Amount", "##amount", &videoSettings->blurSettings.amount, &videoSettings->blurSettings.amountOscillator);
  CommonViews::ModulationSelector(&videoSettings->blurSettings.amount);
  CommonViews::MidiSelector(&videoSettings->blurSettings.amount);
  
  // Radius
  CommonViews::SliderWithOscillator("Radius", "##radius", &videoSettings->blurSettings.radius, &videoSettings->blurSettings.radiusOscillator);
  CommonViews::ModulationSelector(&videoSettings->blurSettings.radius);
  CommonViews::MidiSelector(&videoSettings->blurSettings.radius);

  // Sharpen
  
  CommonViews::H3Title("Sharpen");
  
  CommonViews::SliderWithOscillator("Amount", "##sharpen_amount", &videoSettings->sharpenSettings.amount, &videoSettings->sharpenSettings.amountOscillator);
  CommonViews::ModulationSelector(&videoSettings->sharpenSettings.amount);
  CommonViews::MidiSelector(&videoSettings->sharpenSettings.amount);

  // Radius
  CommonViews::SliderWithOscillator("Radius", "##sharpen_radius", &videoSettings->sharpenSettings.radius, &videoSettings->sharpenSettings.radiusOscillator);
  CommonViews::ModulationSelector(&videoSettings->sharpenSettings.radius);
  CommonViews::MidiSelector(&videoSettings->sharpenSettings.radius);

  // Boost
  CommonViews::SliderWithOscillator("Boost", "##sharpen_boost", &videoSettings->sharpenSettings.boost, &videoSettings->sharpenSettings.boostOscillator);
  CommonViews::ModulationSelector(&videoSettings->sharpenSettings.boost);
  CommonViews::MidiSelector(&videoSettings->sharpenSettings.boost);
}

void VideoSettingsView::drawTransform() {
  CommonViews::H3Title("Transform");

  // Amount
  CommonViews::SliderWithOscillator("Scale", "##scale", &videoSettings->transformSettings.scale, &videoSettings->transformSettings.scaleOscillator);
  CommonViews::ModulationSelector(&videoSettings->transformSettings.scale);
  CommonViews::MidiSelector(&videoSettings->transformSettings.scale);
}

void VideoSettingsView::drawPixelation() {
  CommonViews::H3Title("Pixelation");

  ImGui::Text("Mix");
  ImGui::SetNextItemWidth(150.0);
  ImGui::SliderFloat("##pixelation_mix", &videoSettings->pixelSettings.mix.value, videoSettings->pixelSettings.mix.min, videoSettings->pixelSettings.mix.max, "%.3f");
  ImGui::SameLine(0, 20);
  ImGui::Checkbox("Enabled##pixelation_enabled", &videoSettings->pixelSettings.enabled);
  CommonViews::ModulationSelector(&videoSettings->pixelSettings.mix);
  CommonViews::MidiSelector(&videoSettings->pixelSettings.mix);

  CommonViews::SliderWithOscillator("Scale", "##pixelation_scale", &videoSettings->pixelSettings.scale, &videoSettings->pixelSettings.scaleOscillator);
  CommonViews::ModulationSelector(&videoSettings->pixelSettings.scale);
  CommonViews::MidiSelector(&videoSettings->pixelSettings.scale);
}

