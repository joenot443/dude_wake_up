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
#include "FileBrowserView.hpp"
#include "VideoSettingsView.hpp"
#include "FontService.hpp"
#include "VideoSourceService.hpp"
#include "ShaderChainerService.hpp"

const static ofVec2f windowSize = ofVec2f(1000, 600);

void VideoSettingsView::setup() {
  styleWindow();
  videoSourceBrowserView.setup();
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

void VideoSettingsView::drawMenuBar() {
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
          ConfigService::getService()->saveShaderChainerConfigFile(selectedChainer, result.getPath());
        }
      }
      if (ImGui::MenuItem("Load Config", "CMD+O")) {
        ofFileDialogResult result = ofSystemLoadDialog("Open config file");
        if (result.getPath().length()) {
          auto shaderChainer = std::shared_ptr<ShaderChainer>( ConfigService::getService()->loadShaderChainerConfigFile(result.getPath()));
          if (shaderChainer.get() != nullptr) {
            pushShaderChainer(shaderChainer);
          }
        }
      }
      ImGui::EndMenu();
    }
    ImGui::EndMenuBar();
  }
  
}

void VideoSettingsView::draw() {
  if (!hasDrawn) {
    hasDrawn = true;
  }
  
  ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_MenuBar;
  ImGui::SetNextWindowSize(windowSize);
  auto name = std::string("##%d Video Feed Tabs", videoSettings->streamId);
  
  ImGui::Begin(name.c_str(), NULL, windowFlags);
  
  drawMenuBar();
  
  ImGui::PushFont(FontService::getService()->h2);
  ImGui::Text("Video Feed Settings - %d", videoSettings->streamId);
  ImGui::PopFont();
  ImGui::Separator();
  
  
  
  ImGui::Columns(2, "shaderchainer", false);
  // Draw the shader chainers in different tabs
  drawShaderChainerTabs();
  // Shader Chainer creation
  
  ImGui::NextColumn();
  // Selected Shader
  drawSelectedShader();
  ImGui::Columns(1);
  
  ImGui::Separator();
  
  ImGui::Columns(2, "sourcesettings", false);
//  drawFileBrowserView();
  newVideoSourceView.draw();
  ImGui::NextColumn();
  drawVideoSourceBrowserView();
  ImGui::Columns(1);
  
  // Close Stream button
  if (ImGui::Button("Close Stream")) {
    closeStream(videoSettings->streamId);
  }
  ImGui::End();
}

void VideoSettingsView::selectShaderChainerAtIndex(int i) {
  selectedChainer = shaderChainers[i].get();
  selectedChainerView = shaderChainerViews[i].get();
  videoStream->outputChainer = selectedChainer;
  selectedIndex = i;
}

void VideoSettingsView::drawShaderChainerTabs() {
  
  ImGui::PushFont(FontService::getService()->h3);
  ImGui::Text("Shader Chainers");
  ImGui::PopFont();
  
  if (ImGui::BeginTabBar("Shader Chainers")) {
    for (int i = 0; i < shaderChainers.size(); i++) {
      auto chainer = shaderChainers[i];
      auto chainerView = shaderChainerViews[i];
      if (ImGui::BeginTabItem(chainer->name.c_str())) {
        selectShaderChainerAtIndex(i);
        chainerView->draw();
        ImGui::EndTabItem();
      }
    }
    ImGui::EndTabBar();
  }
}

void VideoSettingsView::drawFileBrowserView() {
  ImGui::PushFont(FontService::getService()->h3);
  ImGui::Text("File Browser");
  ImGui::PopFont();
  ImGui::Separator();
}

void VideoSettingsView::drawVideoSourceBrowserView() {
  videoSourceBrowserView.draw();
}

void VideoSettingsView::drawSelectedShader() {
//  if (selectedChainerView->selectedShader != nullptr) {
//    selectedChainerView->selectedShader->drawSettings();
//  } else {
//    CommonViews::mSpacing();
//    CommonViews::HorizontallyAligned(120);
//    CommonViews::H4Title("Select a Shader");
//    CommonViews::mSpacing();
//  }
}

void VideoSettingsView::pushShaderChainer(std::shared_ptr<ShaderChainer> shaderChainer) {
  ShaderChainerService::getService()->addShaderChainer(shaderChainer);
  shaderChainer->setup();
  shaderChainers.push_back(shaderChainer);
  auto shaderChainerView = std::make_shared<ShaderChainerView>(shaderChainer);
  shaderChainerViews.push_back(shaderChainerView);
  selectShaderChainerAtIndex(shaderChainers.size() - 1);
}
