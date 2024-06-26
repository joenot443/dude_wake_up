//
//  MainStageView.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 11/14/22.
//

#include <sentry.h>
#include "ShaderInfoView.hpp"
#include "CommonStrings.hpp"
#include "MainStageView.hpp"
#include "ConfigService.hpp"
#include "TileBrowserView.hpp"
#include "ParameterTileBrowserView.hpp"
#include "MarkdownView.hpp"
#include "FontService.hpp"
#include "HSBShader.hpp"
#include "LayoutStateService.hpp"
#include "NodeLayoutView.hpp"
#include "OscillationService.hpp"
#include "ParameterService.hpp"
#include "ShaderChainerService.hpp"
#include "ShaderSettings.hpp"
#include "ShaderType.hpp"
#include "UUID.hpp"
#include "SubmitFeedbackView.hpp"
#include "VideoSourceService.hpp"
#include "implot.h"

static const ImVec2 ShaderButtonSize = ImVec2(90, 30);
static const float MenuBarHeight = 50.0f;

void MainStageView::setup()
{
  NodeLayoutView::getInstance()->setup();
  shaderBrowserView.setup();
  strandBrowserView.setup();
  videoSourceBrowserView.setup();
  audioSourceBrowserView.setup();
  stageModeView.setup();
}

void MainStageView::update()
{
  videoSourceBrowserView.update();
  audioSourceBrowserView.update();
  shaderBrowserView.update();
  stageModeView.update();
  NodeLayoutView::getInstance()->update();
}

void MainStageView::draw()
{
  // Draw a table with 2 columns, sized | 1/5 |   4/5   |
  
  float nodeLayoutWidth = LayoutStateService::getService()->shouldDrawShaderInfo() ?  (getScaledWindowWidth() * 3) / 5 : (getScaledWindowWidth() * 4) / 5;
  float nodeLayoutHeight = getScaledWindowHeight() - LayoutStateService::getService()->audioSettingsViewHeight();
  
//  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(15.0, 15.0));
  int columnCount = LayoutStateService::getService()->shouldDrawShaderInfo() ? 3 : 2;
  ImGui::Columns(columnCount, "main_stage_view", false);
  ImGui::SetColumnWidth(0, getScaledWindowWidth() / 5);
  ImGui::SetColumnWidth(1, nodeLayoutWidth);
  if (LayoutStateService::getService()->shouldDrawShaderInfo()) {
    ImGui::SetColumnWidth(2, getScaledWindowWidth() / 5);
  }
  
  
  // | Sources |  Node Layout
  // | Effects |        ''
  // | Library |       Audio
  
  // Sources
  auto browserSize = ImVec2(ImGui::GetWindowContentRegionMax().x / 5.,
                            (ImGui::GetWindowContentRegionMax().y - MenuBarHeight) / 3.);
//  ImGui::PopStyleVar();
  ImGui::BeginChild("##sourceBrowser", browserSize, false, ImGuiWindowFlags_AlwaysUseWindowPadding);
  CommonViews::H3Title("Sources");
  drawVideoSourceBrowser();
  ImGui::EndChild();

  ImGui::BeginChild("##shaderBrowser", browserSize, false, ImGuiWindowFlags_AlwaysUseWindowPadding);
  CommonViews::H3Title("Effects");
  drawShaderBrowser();
  ImGui::EndChild();

  ImGui::BeginChild("##libraryBrowser", browserSize, false, ImGuiWindowFlags_AlwaysUseWindowPadding);
  CommonViews::H3Title("Saved Strands");
  strandBrowserView.draw();
  ImGui::EndChild();

  drawMenu();
  
  ImGui::NextColumn();
  
  // NodeLayout OR StageMode
  if (LayoutStateService::getService()->stageModeEnabled) {
    stageModeView.draw();
    audioSourceBrowserView.draw();
  } else {
    NodeLayoutView::getInstance()->draw();
    audioSourceBrowserView.draw();
  }
  
  
  // Shader Info
  if (LayoutStateService::getService()->shouldDrawShaderInfo()) {
    auto shaderInfoPaneSize = ImVec2(ImGui::GetWindowContentRegionMax().x / 5.,(ImGui::GetWindowContentRegionMax().y - MenuBarHeight));
    
    ImGui::NextColumn();
    ImGui::BeginChild("##shaderInfo", shaderInfoPaneSize, false, ImGuiWindowFlags_AlwaysUseWindowPadding);
    ShaderInfoView::draw();
    ImGui::EndChild();
  }
}

void MainStageView::drawMenu()
{
  if (ImGui::BeginMenuBar())
  {
    // Save Config
    if (ImGui::BeginMenu("File"))
    {
      if (ImGui::MenuItem("Save", "Cmd+S", false, ConfigService::getService()->isEditingWorkspace())) {
        ConfigService::getService()->saveCurrentWorkspace();
      }
      
      if (ImGui::MenuItem("Save Workspace As", "Cmd+Shift+S"))
      {
        ConfigService::getService()->saveNewWorkspace();
      }
      if (ImGui::MenuItem("Load Workspace", "Cmd+O"))
      {
        ConfigService::getService()->loadWorkspaceDialogue();
      }
      if (ImGui::MenuItem("New Workspace", "Cmd+N"))
      {
        ShaderChainerService::getService()->clear();
        VideoSourceService::getService()->clear();
        ConfigService::getService()->closeWorkspace();
      }
      ImGui::EndMenu();
    }
    
    if (ImGui::BeginMenu("Edit"))
    {
      if (ImGui::MenuItem("Clear Feedback Buffers", "Cmd+K"))
      {
        FeedbackSourceService::getService()->clearBuffers();
      }
      ImGui::EndMenu();
    }
    
    if (ImGui::BeginMenu("View"))
    {
      std::string title = LayoutStateService::getService()->midiEnabled ? CommonStrings::DisableMidi : CommonStrings::EnableMidi;
      if (ImGui::MenuItem(title.c_str()))
      {
        LayoutStateService::getService()->midiEnabled = !LayoutStateService::getService()->midiEnabled;
      }
      if (ImGui::MenuItem("Toggle Stage Mode", "Cmd+B"))
      {
        LayoutStateService::getService()->stageModeEnabled = !LayoutStateService::getService()->stageModeEnabled;
      }
      if (ImGui::MenuItem("Toggle Shader Info View", "Cmd+T"))
      {
        LayoutStateService::getService()->shaderInfoEnabled = !LayoutStateService::getService()->shaderInfoEnabled;
      }
      ImGui::EndMenu();
    }
    
    if (ImGui::MenuItem("Save Default Workspace"))
    {
      ConfigService::getService()->saveDefaultConfigFile();
    }
    if (ImGui::MenuItem("Load Default Workspace"))
    {
      ConfigService::getService()->loadDefaultConfigFile();
    }
    
    if (ImGui::BeginPopupModal(SubmitFeedbackView::popupId, nullptr, ImGuiPopupFlags_MouseButtonLeft))
    {
      submitFeedbackView.draw();
      ImGui::EndPopup();
    }
    // Present the submit feedback view in a popup modal when the menu button is pressed
    if (ImGui::MenuItem("Submit Feedback"))
    {
      ImGui::OpenPopup(SubmitFeedbackView::popupId);
    }
    
    static bool showingMenu = false;
    if (ImGui::MenuItem("ImGui Demo") || showingMenu)
    {
      ImGui::ShowDemoWindow();
      showingMenu = true;
    }
    
    ImGui::EndMenuBar();
  }
}

void MainStageView::drawMasks() {
  ImVec2 windowSize = ofGetWindowSize();
  float audioViewHeight = LayoutStateService::getService()->audioSettingsViewHeight();
  ofRectangle browserRect = ofRectangle(0, 0, windowSize.x / 5.0, windowSize.y);
  ofRectangle audioRect = ofRectangle(0, windowSize.y - audioViewHeight, windowSize.x, audioViewHeight);
  auto cursor = ImGui::GetCursorPos();
  ImGui::SetCursorPos(ImVec2(0,0));
  ImGui::SetNextWindowSize(ImVec2(windowSize.x / 5.0, windowSize.y));
  ImGui::End();
  ImGui::SetCursorPos(cursor);
}

void MainStageView::drawVideoSourceBrowser()
{
  videoSourceBrowserView.draw();
}

void MainStageView::drawShaderBrowser() { shaderBrowserView.draw(); }

void MainStageView::keyReleased(int key)
{
  
  NodeLayoutView::getInstance()->keyReleased(key);
  
  // If Space is pressed, tap the bpmTapper
  if (key == ' ')
  {
    bpmTapper.tap();
  }
  
  // Cmd Based Key Binds
  
  if (ofGetKeyPressed(OF_KEY_SUPER)) {
    // Save Workspace
    if (key == 's') {
      ConfigService::getService()->saveCurrentWorkspace();
    }
    // Load Workspace
    if (key == 'o') {
      ConfigService::getService()->loadWorkspaceDialogue();
    }
    
    // New Workspace
    if (key == 'n') {
      ShaderChainerService::getService()->clear();
      VideoSourceService::getService()->clear();
      ConfigService::getService()->closeWorkspace();
    }
    
    // Clear buffers
    if (key == 'k') {
      FeedbackSourceService::getService()->clearBuffers();
    }
    
    // Enable Stage mode
    if (key == 'b') {
      LayoutStateService::getService()->stageModeEnabled = !LayoutStateService::getService()->stageModeEnabled;
    }
    
    // Enable Shader Info pane
    if (key == 't') {
      LayoutStateService::getService()->shaderInfoEnabled = !LayoutStateService::getService()->shaderInfoEnabled;
    }
  }
}
