//
//  MainStageView.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 11/14/22.
//

#include <sentry.h>
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
  float width = ImGui::GetWindowContentRegionMax().x;
  float height = ImGui::GetWindowContentRegionMax().y;
  
  float nodeLayoutWidth = (getScaledWindowWidth() * 4) / 5;
  float nodeLayoutHeight = getScaledWindowHeight() - LayoutStateService::getService()->audioSettingsViewHeight();
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(15.0, 15.0));
  ImGui::Columns(2, "main_stage_view", false);
  ImGui::SetColumnWidth(0, getScaledWindowWidth() / 5);
  ImGui::SetColumnWidth(1, (getScaledWindowWidth() * 4) / 5);
  
  // | Sources |  Node Layout
  // | Effects |        ''
  // | Library |       Audio
  
  // Sources
  auto browserSize = ImVec2(ImGui::GetWindowContentRegionMax().x / 5.,
                            (ImGui::GetWindowContentRegionMax().y - MenuBarHeight) / 3.);
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
  
  ImGui::PopStyleVar();
}

void MainStageView::drawMenu()
{
  if (ImGui::BeginMenuBar())
  {
    // Save Config
    if (ImGui::BeginMenu("File"))
    {
      if (ImGui::MenuItem("Save", "", false, ConfigService::getService()->isEditingWorkspace())) {
        ConfigService::getService()->saveCurrentWorkspace();
      }
      
      if (ImGui::MenuItem("Save Workspace As"))
      {
        // Present a file dialog to save the config file
        // Use a default name of "CURRENT_DATE_TIME.json"
        std::string defaultName =
        ofGetTimestampString("Workspace-%m-%d-%Y.json");
        ofFileDialogResult result =
        ofSystemSaveDialog(defaultName, "Save Workspace");
        if (result.bSuccess)
        {
          ConfigService::getService()->saveWorkspace(std::make_shared<Workspace>(result.getName(), result.getPath()));
        }
      }
      if (ImGui::MenuItem("Load Workspace"))
      {
        // Present a file dialog to load the config file
        ofFileDialogResult result = ofSystemLoadDialog("Open Workspace", false);
        if (result.bSuccess)
        {
          ConfigService::getService()->loadWorkspace(std::make_shared<Workspace>(result.getName(), result.getPath()));
        }
      }
      if (ImGui::MenuItem("Reset Stage"))
      {
        ShaderChainerService::getService()->clear();
        VideoSourceService::getService()->clear();
        ConfigService::getService()->closeWorkspace();
      }
      ImGui::EndMenu();
    }
    
    if (ImGui::BeginMenu("Effects"))
    {
      if (ImGui::MenuItem("Clear Feedback Buffers (C)"))
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
      if (ImGui::MenuItem("Toggle Stage Mode", "V"))
      {
        LayoutStateService::getService()->stageModeEnabled = !LayoutStateService::getService()->stageModeEnabled;
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
  
  if (key == 'c') {
    FeedbackSourceService::getService()->clearBuffers();
  }
  
  if (key == 'v') {
    LayoutStateService::getService()->stageModeEnabled = !LayoutStateService::getService()->stageModeEnabled;
  }
}
