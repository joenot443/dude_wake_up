//
//  ShaderChainerSettingsView.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 11/20/22.
//

#include "ShaderChainerSettingsView.hpp"
#include "VideoSourceService.hpp"
#include "ShaderChainerService.hpp"
#include "CommonViews.hpp"
#include "imgui_stdlib.h"

void ShaderChainerSettingsView::setup() {
}

void ShaderChainerSettingsView::update() {
  shaderChainer = ShaderChainerService::getService()->selectedShaderChainer;
}

void ShaderChainerSettingsView::draw() {
  CommonViews::H4Title("Shader Chainer Settings");
  drawNameField();
  drawVideoSourceSelector();
  drawVideoSourceSettings();
}

// Draw an ImGui text field for the shaderChainer's name
void ShaderChainerSettingsView::drawNameField() {
//  static char buf[128] = "";
//  // Populate buf with shaderChainer->videoSource->sourceName
////  strcpy(buf, shaderChainer->name.c_str());
  ImGui::InputText("Name", &shaderChainer->name);
}

// Draw an ImGui combo box for the shaderChainer's videoSource
void ShaderChainerSettingsView::drawVideoSourceSelector() {
  std::vector<std::string> videoSourceNames;
  for (auto videoSource : VideoSourceService::getService()->videoSources()) {
    videoSourceNames.push_back(videoSource->sourceName);
  }
  
  int videoSourceIndex = 0;
  for (int i = 0; i < videoSourceNames.size(); i++) {
    if (videoSourceNames[i] == shaderChainer->source->sourceName) {
      videoSourceIndex = i;
    }
  }
  
  if (ImGui::BeginCombo("Video Source", shaderChainer->source->sourceName.c_str())) {
    for (int i = 0; i < videoSourceNames.size(); i++) {
      bool isSelected = (videoSourceIndex == i);
      if (ImGui::Selectable(videoSourceNames[i].c_str(), isSelected)) {
        videoSourceIndex = i;
        shaderChainer->source = VideoSourceService::getService()->videoSources()[i];
      }
      if (isSelected) {
        ImGui::SetItemDefaultFocus();
      }
    }
    ImGui::EndCombo();
  }
}

void ShaderChainerSettingsView::drawVideoSourceSettings() {
  shaderChainer->source->drawSettings();
}
