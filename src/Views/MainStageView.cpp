//
//  MainStageView.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 11/14/22.
//

#include "MainStageView.hpp"
#include "VideoSourceService.hpp"
#include "UUID.hpp"
#include "ShaderChainerService.hpp"
#include "OscillationService.hpp"
#include "FontService.hpp"
#include "ShaderChainerView.hpp"
#include "ShaderType.hpp"
#include "HSBShader.hpp"

void MainStageView::setup() {
  OscillationService::getService()->loadConfigFile();

  VideoSourceService::getService()->addWebcamVideoSource("Webcam 0", 1);
  VideoSourceService::getService()->addShaderVideoSource(ShaderSource_plasma);
  VideoSourceService::getService()->addShaderVideoSource(ShaderSource_fractal);
  VideoSourceService::getService()->addShaderVideoSource(ShaderSource_fuji);
  
  
  // Create a ShaderChainer with first video source
  auto shaderChainer = std::make_shared<ShaderChainer>(UUID::generateUUID(), "Main Chainer", VideoSourceService::getService()->videoSources().at(0));
  
  shaderChainer->setup();
  // Add an HSB shader to the ShaderChainer
  shaderChainer->pushShader(ShaderTypeHSB);
  
  // Add the ShaderChainer to the ShaderChainerService
  ShaderChainerService::getService()->addShaderChainer(shaderChainer);
  
  // Create a ShaderChainerView for the ShaderChainer
  auto shaderChainerView = std::make_shared<ShaderChainerView>(shaderChainer);
  shaderChainerViews.push_back(shaderChainerView);
  
  videoSourceBrowserView.setup();
  videoSourcePreviewView.setup();
  outputBrowserView.setup();
  shaderChainerSettingsView.setup();
  
}

void MainStageView::update() {
  videoSourceBrowserView.update();
  videoSourcePreviewView.update();
  outputBrowserView.update();
  shaderChainerSettingsView.update();
  
  if (videoSourceBrowserView.selectedVideoSource) {
    videoSourcePreviewView.videoSource = videoSourceBrowserView.selectedVideoSource;
  }
}

void MainStageView::draw() {
  // Draw a table with 3 columns, sized 1/6, 1/2, 1/3
  ImGui::Columns(3, "main_stage_view", false);
  ImGui::SetColumnWidth(0, ImGui::GetWindowWidth() / 6);
  ImGui::SetColumnWidth(1, ImGui::GetWindowWidth() / 2);
  ImGui::SetColumnWidth(2, ImGui::GetWindowWidth() / 3);
  
  // | Sources |   ShaderChainers   | Outputs
  // |               New Chainer
  // |         | Shader |    Osc    |          |
  
  
  
  // Sources
  drawVideoSourceBrowser();
  
  ImGui::NextColumn();
  
  // Chainers
  drawShaderChainers();
  drawNewShaderChainerButton();
  drawShaderSelection();
  
  drawSelectedShader();
  
  ImGui::NextColumn();
  
  // Outputs
  drawOutputBrowser();
}

void MainStageView::drawSelectedShader() {
  ImGui::BeginChild("selected_shader");
  if (ImGui::BeginTable("##shaderTable", 2)) {
    ImGui::TableNextColumn();
    
    if (ShaderChainerService::getService()->selectedShader != nullptr) {
      ShaderChainerService::getService()->selectedShader->drawSettings();
    } else {
      drawShaderChainerSettings();
    }
    
    ImGui::TableNextColumn();
    
    oscillatorView.draw();
    
    ImGui::EndTable();
  }
  ImGui::EndChild();
}

void MainStageView::drawShaderChainerSettings() {
  ImGui::Text("Shader Chainer Settings");
  shaderChainerSettingsView.draw();
}

void MainStageView::drawNewShaderButton() {
  auto selectedShaderName = shaderTypeName(selectedShaderType);
  auto selectedShaderChainer = ShaderChainerService::getService()->selectedShaderChainer;
  
  if (ImGui::BeginCombo("ShaderCombo", selectedShaderName.c_str()))
  {
    for (auto shaderType : AvailableShaderTypes) {
      if (ImGui::Selectable(shaderTypeName(shaderType).c_str(), selectedShaderType == shaderType))
        selectedShaderType = shaderType;
    }
    
    ImGui::EndCombo();
  }
  
  ImGui::SameLine();
  
  if (ImGui::Button("Add New Shader") && selectedShaderType != ShaderTypeNone) {
    selectedShaderChainer->pushShader(selectedShaderType);
  }
}

void MainStageView::drawNewShaderChainerButton() {
  ImGui::PushFont(FontService::getService()->h4);
  ImGui::Text("New Shader Chain");
  ImGui::PopFont();
  
  ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.3f);
  static char buf[128] = "";
  ImGui::InputText("##ShaderChainerName", buf, IM_ARRAYSIZE(buf));
  ImGui::PopItemWidth();
  ImGui::SameLine();
  
  if (ImGui::Button("Create")) {
    auto name = std::string(buf);
    if (name.empty()) {
      name = formatString("Chainer - %d", shaderChainerViews.size());
    }
    auto videoSources = VideoSourceService::getService()->videoSources();
    
    auto shaderChainer = std::make_shared<ShaderChainer>(UUID::generateUUID(), name, videoSources.at(0));
    shaderChainer->setup();
    pushShaderChainer(shaderChainer);
  }
}

void MainStageView::pushShaderChainer(std::shared_ptr<ShaderChainer> shaderChainer)
{
  ShaderChainerService::getService()->addShaderChainer(shaderChainer);
  shaderChainer->setup();
  auto shaderChainerView = std::make_shared<ShaderChainerView>(shaderChainer);
  shaderChainerViews.push_back(shaderChainerView);
  
  // Sort the ShaderChainerViews by their names
  std::sort(shaderChainerViews.begin(), shaderChainerViews.end(), [](const std::shared_ptr<ShaderChainerView> &a, const std::shared_ptr<ShaderChainerView> &b) {
    return a->shaderChainer->name < b->shaderChainer->name;
  });
  
}

void MainStageView::drawVideoSourceBrowser() {
  videoSourceBrowserView.draw();
  videoSourcePreviewView.draw();
}

void MainStageView::drawOutputBrowser() {
  outputBrowserView.draw();
}

void MainStageView::drawShaderChainers() {
  // Iterate through the shader chainers
  for (auto shaderChainerView : shaderChainerViews) {
    shaderChainerView->draw();
  }
}

void MainStageView::drawShaderSelection() {
  for (auto shaderType : AvailableShaderTypes) {
    ImGui::Button(formatString("%s", shaderTypeName(shaderType).c_str()).c_str(), ImVec2(60, 30));
    // Our buttons are both drag sources and drag targets here!
    if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
    {
      // Set payload to carry the index of our item (could be anything)
      ImGui::SetDragDropPayload("NewShader", &shaderType, sizeof(ShaderType));
      
      ImGui::Text("%s", shaderTypeName(shaderType).c_str());
      ImGui::EndDragDropSource();
    }
    
    ImGui::SameLine();
  }
  ImGui::Text("Drag a Shader to the Chainer");
}


