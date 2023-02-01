//
//  MainStageView.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 11/14/22.
//

#include "MainStageView.hpp"
#include "implot.h"
#include "ConfigService.hpp"
#include "FontService.hpp"
#include "HSBShader.hpp"
#include "NodeLayoutView.hpp"
#include "OscillationService.hpp"
#include "LayoutStateService.hpp"
#include "ParameterService.hpp"
#include "ShaderChainerService.hpp"
#include "ShaderChainerView.hpp"
#include "ShaderSettings.hpp"
#include "ShaderType.hpp"
#include "UUID.hpp"
#include "VideoSourceService.hpp"
#include "implot.h"

static const ImVec2 ShaderButtonSize = ImVec2(90, 30);

void MainStageView::setup() {
  nodeLayoutView.setup();
  shaderBrowserView.setup();
  populateShaderChainerViews();
  videoSourceBrowserView.setup();
  videoSourcePreviewView.setup();
  outputBrowserView.setup();
  shaderChainerSettingsView.setup();
  shaderChainerStageView.setup();
  audioSourceBrowserView.setup();
}

void MainStageView::update() {
  videoSourceBrowserView.update();
  videoSourcePreviewView.update();
  outputBrowserView.update();
  shaderChainerSettingsView.update();
  shaderChainerStageView.update();
  audioSourceBrowserView.update();
  nodeLayoutView.update();
}

void MainStageView::draw() {
  // Draw a table with 3 columns, sized 1/5, 3/5, 1/5
  ImGui::Columns(3, "main_stage_view", false);
  ImGui::SetColumnWidth(0, ImGui::GetWindowWidth() / 5.);
  ImGui::SetColumnWidth(1, 3. * (ImGui::GetWindowWidth() / 5.));
  ImGui::SetColumnWidth(2, ImGui::GetWindowWidth() / 5.);

  // | Sources |  Node Layout   | Outputs
  // | Effects |        ''      |   ''
  // | Library |       Audio    |   ''

  // Sources
  auto browserSize = ImVec2(ImGui::GetWindowContentRegionMax().x / 5., ImGui::GetWindowContentRegionMax().y / 3.);
  
  ImGui::BeginChild("##sourceBrowser", browserSize);
  drawVideoSourceBrowser();
  ImGui::EndChild();
  
  ImGui::BeginChild("##shaderBrowser", browserSize);
  drawShaderBrowser();
  ImGui::EndChild();
  
  ImGui::BeginChild("##libraryBrowser", browserSize);
  fileBrowserView.draw();
  ImGui::EndChild();
  
  drawMenu();
  

  ImGui::NextColumn();

  // Chainers
  nodeLayoutView.draw();

  audioSourceBrowserView.draw();
  
  ImGui::NextColumn();

  // Outputs
  drawOutputBrowser();
}

void MainStageView::drawMenu() {
  if (ImGui::BeginMenuBar()) {
    // Save Config
    if (ImGui::BeginMenu("File")) {
      if (ImGui::MenuItem("Save Config")) {
        // Present a file dialog to save the config file
        // Use a default name of "CURRENT_DATE_TIME.json"
        std::string defaultName =
            ofGetTimestampString("%Y-%m-%d_%H-%M-%S.json");
        ofFileDialogResult result =
            ofSystemSaveDialog(defaultName, "Save File");
        if (result.bSuccess) {
          ConfigService::getService()->saveConfigFile(result.getPath());
        }
      }
      if (ImGui::MenuItem("Load Config")) {
        // Present a file dialog to load the config file
        ofFileDialogResult result = ofSystemLoadDialog("Open File", false);
        if (result.bSuccess) {
          ConfigService::getService()->loadConfigFile(result.getPath());
        }
      }
      ImGui::EndMenu();
    }

    if (ImGui::MenuItem("Save Default Config")) {
      ConfigService::getService()->saveDefaultConfigFile();
    }
    if (ImGui::MenuItem("Load Default Config")) {
      ConfigService::getService()->loadDefaultConfigFile();
    }
    ImGui::EndMenuBar();
  }
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

    OscillatorView::draw(OscillationService::getService()->selectedOscillator,
                         ParameterService::getService()->selectedParameter);

    ImGui::EndTable();
  }
  ImGui::EndChild();
}

void MainStageView::drawShaderChainerSettings() {
  ImGui::Text("Shader Chainer Settings");
  if (shaderChainerSettingsView.shaderChainer != nullptr) {
    shaderChainerSettingsView.draw();
  }
}

void MainStageView::drawNewShaderButton() {
  auto selectedShaderName = shaderTypeName(selectedShaderType);
  auto selectedShaderChainer =
      ShaderChainerService::getService()->selectedShaderChainer;

  if (ImGui::BeginCombo("ShaderCombo", selectedShaderName.c_str())) {
    for (auto shaderType : AvailableShaderTypes) {
      if (ImGui::Selectable(shaderTypeName(shaderType).c_str(),
                            selectedShaderType == shaderType))
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

    auto shaderChainer = std::make_shared<ShaderChainer>(
        UUID::generateUUID(), name, videoSources.at(0));
    shaderChainer->setup();
    pushShaderChainer(shaderChainer);
  }
}

void MainStageView::populateShaderChainerViews() {
  shaderChainerViews.clear();

  auto chainers = ShaderChainerService::getService()->shaderChainers();

  for (auto c : chainers) {
    auto shaderChainerView = std::make_shared<ShaderChainerView>(c);
    shaderChainerViews.push_back(shaderChainerView);
  }

  std::sort(shaderChainerViews.begin(), shaderChainerViews.end(),
            [](const std::shared_ptr<ShaderChainerView> &a,
               const std::shared_ptr<ShaderChainerView> &b) {
              return a->shaderChainer->name < b->shaderChainer->name;
            });
}

void MainStageView::pushShaderChainer(
    std::shared_ptr<ShaderChainer> shaderChainer) {
  ShaderChainerService::getService()->addShaderChainer(shaderChainer);
  shaderChainer->setup();
  auto shaderChainerView = std::make_shared<ShaderChainerView>(shaderChainer);
  shaderChainerViews.push_back(shaderChainerView);

  // Sort the ShaderChainerViews by their names
  std::sort(shaderChainerViews.begin(), shaderChainerViews.end(),
            [](const std::shared_ptr<ShaderChainerView> &a,
               const std::shared_ptr<ShaderChainerView> &b) {
              return a->shaderChainer->name < b->shaderChainer->name;
            });
}

void MainStageView::drawVideoSourceBrowser() {
  videoSourceBrowserView.draw();
  videoSourcePreviewView.draw();
}

void MainStageView::drawOutputBrowser() { outputBrowserView.draw(); }

void MainStageView::drawShaderBrowser() {
  shaderBrowserView.draw();
}

void MainStageView::keyReleased(int key) { nodeLayoutView.keyReleased(key); }
