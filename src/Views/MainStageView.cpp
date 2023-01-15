//
//  MainStageView.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 11/14/22.
//

#include "MainStageView.hpp"
#include "ConfigService.hpp"
#include "FontService.hpp"
#include "HSBShader.hpp"
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

void MainStageView::setup() {
    VideoSourceService::getService()->addWebcamVideoSource("Webcam 0", 0);
  VideoSourceService::getService()->addWebcamVideoSource("Webcam 1", 1);
    VideoSourceService::getService()->addWebcamVideoSource("Webcam 2", 2);
  VideoSourceService::getService()->addShaderVideoSource(ShaderSource_plasma);
  VideoSourceService::getService()->addShaderVideoSource(ShaderSource_fractal);
  VideoSourceService::getService()->addShaderVideoSource(ShaderSource_fuji);
  VideoSourceService::getService()->addShaderVideoSource(ShaderSource_clouds);
  VideoSourceService::getService()->addShaderVideoSource(
      ShaderSource_Mountains);
  VideoSourceService::getService()->addShaderVideoSource(
      ShaderSource_audioMountains);

  nodeLayoutView.setup();

  VideoSourceService::getService()->addShaderVideoSource(ShaderSource_melter);
  VideoSourceService::getService()->addShaderVideoSource(ShaderSource_rings);
//  VideoSourceService::getService()->addShaderVideoSource(ShaderSource_Rubiks);
  VideoSourceService::getService()->addShaderVideoSource(
      ShaderSource_audioBumper);
  VideoSourceService::getService()->addShaderVideoSource(
      ShaderSource_audioWaveform);
  VideoSourceService::getService()->addShaderVideoSource(ShaderSource_galaxy);

  //  ConfigService::getService()->loadDefaultConfigFile();
  populateShaderChainerViews();

  //  // Create a ShaderChainer with first video source
  auto shaderChainer = std::make_shared<ShaderChainer>(
      UUID::generateUUID(), "Main Chainer",
      VideoSourceService::getService()->videoSources().at(0));

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

//  if (videoSourceBrowserView.selectedVideoSource) {
//    videoSourcePreviewView.videoSource =
//        videoSourceBrowserView.selectedVideoSource;
//  }
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
  audioSourceBrowserView.draw();
  drawMenu();

  ImGui::NextColumn();

  // Chainers
  shaderChainerStageView.draw();
  nodeLayoutView.draw();
  drawNewShaderChainerButton();
  drawShaderSelection();

  drawSelectedShader();

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

void MainStageView::drawShaderSelection() {
  float maxX = ImGui::GetWindowWidth() * (4. / 6.);
  auto n = 0;

  for (auto shaderType : AvailableShaderTypes) {
    ImGui::Button(
        formatString("%s", shaderTypeName(shaderType).c_str()).c_str(),
        ShaderButtonSize);
    float nextX = ImGui::GetItemRectMax().x + ImGui::GetStyle().ItemSpacing.x +
                  ShaderButtonSize.x;

    if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
      // Set payload to carry the index of our item (could be anything)
      ImGui::SetDragDropPayload("NewShader", &shaderType, sizeof(ShaderType));

      ImGui::Text("%s", shaderTypeName(shaderType).c_str());
      ImGui::EndDragDropSource();
    }

    if (n + 1 < sizeof(AvailableShaderTypes) && nextX < maxX) {
      ImGui::SameLine();
    }
    n += 1;
  }

  ImGui::Text("Drag a Shader to the Chainer");
}

void MainStageView::keyReleased(int key) { nodeLayoutView.keyReleased(key); }
