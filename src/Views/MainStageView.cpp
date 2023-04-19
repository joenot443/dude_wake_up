//
//  MainStageView.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 11/14/22.
//

#include "MainStageView.hpp"
#include "ConfigService.hpp"
#include "TileBrowserView.hpp"
#include "MarkdownView.hpp"
#include "FontService.hpp"
#include "HSBShader.hpp"
#include "LayoutStateService.hpp"
#include "NodeLayoutView.hpp"
#include "OscillationService.hpp"
#include "ParameterService.hpp"
#include "ShaderChainerService.hpp"
#include "ShaderChainerView.hpp"
#include "ShaderSettings.hpp"
#include "ShaderType.hpp"
#include "UUID.hpp"
#include "VideoSourceService.hpp"
#include "implot.h"

static const ImVec2 ShaderButtonSize = ImVec2(90, 30);
static const float MenuBarHeight = 50.0f;

void MainStageView::setup() {
  NodeLayoutView::getInstance()->setup();
  shaderBrowserView.setup();
  fileBrowserView.setup();
  videoSourceBrowserView.setup();
  outputBrowserView.setup();
  shaderChainerSettingsView.setup();
  shaderChainerStageView.setup();
  audioSourceBrowserView.setup();
}

void MainStageView::update() {
  videoSourceBrowserView.update();
  outputBrowserView.update();
  shaderChainerSettingsView.update();
  shaderChainerStageView.update();
  audioSourceBrowserView.update();
  NodeLayoutView::getInstance()->update();
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
  auto browserSize = ImVec2(ImGui::GetWindowContentRegionMax().x / 5.,
                            (ImGui::GetWindowContentRegionMax().y - MenuBarHeight) / 3.);

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
  NodeLayoutView::getInstance()->draw();

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

void MainStageView::drawVideoSourceBrowser() {
  videoSourceBrowserView.draw();
}

void MainStageView::drawOutputBrowser() { outputBrowserView.draw(); }

void MainStageView::drawShaderBrowser() { shaderBrowserView.draw(); }

void MainStageView::keyReleased(int key) { NodeLayoutView::getInstance()->keyReleased(key); }
