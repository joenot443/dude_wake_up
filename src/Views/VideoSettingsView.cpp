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

const static ofVec2f windowSize = ofVec2f(1000, 600);

void VideoSettingsView::setup() {
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
          ConfigService::getService()->saveShaderChainerConfigFile(videoStream->shaderChainer, result.getPath());
        }
      }
      if (ImGui::MenuItem("Load Config", "CMD+O")) {
        ofFileDialogResult result = ofSystemLoadDialog("Open config file");
        if (result.getPath().length()) {
          auto shaderChainer = ConfigService::getService()->loadShaderChainerConfigFile(result.getPath());
          if (shaderChainer != nullptr) {
            shaderChainerView.setShaderChainer(shaderChainer);
            videoStream->shaderChainer = shaderChainer;
          }
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
  
  
  ImGui::Columns(2, "videofeedsettings", false);
  
  // Shader Chainer
  shaderChainerView.draw();
  ImGui::NextColumn();
  
  // Selected Shader
  drawSelectedShader();
  
  ImGui::Columns(1);
  ImGui::Separator();
  
  // Close Stream button
  if (ImGui::Button("Close Stream")) {
    closeStream(videoSettings->streamId);
  }
  ImGui::End();
}

void VideoSettingsView::drawMenu() {
  
}

void VideoSettingsView::drawSelectedShader() {
  if (shaderChainerView.selectedShader) {
    shaderChainerView.selectedShader->drawSettings();
  } else {
    CommonViews::mSpacing();
    CommonViews::HorizontallyAligned(120);
    CommonViews::H4Title("Select a Shader");
    CommonViews::mSpacing();
  }
  
}
//
//void VideoSettingsView::drawTransform() {
//  CommonViews::H3Title("Transform");
//
//  // Feedback Blend
//  CommonViews::SliderWithOscillator("Feedback Blend", "##feedbackblend", &videoSettings->transformSettings.feedbackBlend, &videoSettings->transformSettings.feedbackBlendOscillator);
//  CommonViews::ModulationSelector(&videoSettings->transformSettings.feedbackBlend);
//  CommonViews::MidiSelector(&videoSettings->transformSettings.feedbackBlend);
//
//
//  // Scale
//  CommonViews::SliderWithOscillator("Scale", "##scale", &videoSettings->transformSettings.scale, &videoSettings->transformSettings.scaleOscillator);
//  CommonViews::ModulationSelector(&videoSettings->transformSettings.scale);
//  CommonViews::MidiSelector(&videoSettings->transformSettings.scale);
//}
