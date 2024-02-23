//
//  MainStageView.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 11/14/22.
//

#include <sentry.h>
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
  fileBrowserView.setup();
  videoSourceBrowserView.setup();
  audioSourceBrowserView.setup();
}

void MainStageView::update()
{
  videoSourceBrowserView.update();
  audioSourceBrowserView.update();
  NodeLayoutView::getInstance()->update();
}

void MainStageView::draw()
{
  // Draw a table with 2 columns, sized | 1/5 |   4/5   |

  ImGui::Columns(2, "main_stage_view", false);
  ImGui::SetColumnWidth(0, ImGui::GetWindowWidth() / 5.);
  ImGui::SetColumnWidth(1, 4. * (ImGui::GetWindowWidth() / 5.));

  // | Sources |  Node Layout
  // | Effects |        ''
  // | Library |       Audio

  // Sources
  auto browserSize = ImVec2(ImGui::GetWindowContentRegionMax().x / 5.,
                            (ImGui::GetWindowContentRegionMax().y - MenuBarHeight) / 3.);
  ImGui::BeginChild("##sourceBrowser", browserSize);
  CommonViews::H3Title("Sources");
  drawVideoSourceBrowser();
  ImGui::EndChild();

  ImGui::BeginChild("##shaderBrowser", browserSize);
  CommonViews::H3Title("Effects");
  drawShaderBrowser();
  ImGui::EndChild();

  ImGui::BeginChild("##libraryBrowser", browserSize);
  CommonViews::H3Title("Saved Chains");
  fileBrowserView.draw();
  ImGui::EndChild();

  drawMenu();

  ImGui::NextColumn();

  // NodeLayout
  NodeLayoutView::getInstance()->draw();

  audioSourceBrowserView.draw();
}

void MainStageView::drawMenu()
{
  if (ImGui::BeginMenuBar())
  {
    // Save Config
    if (ImGui::BeginMenu("File"))
    {
      if (ImGui::MenuItem("Save Config"))
      {
        // Present a file dialog to save the config file
        // Use a default name of "CURRENT_DATE_TIME.json"
        std::string defaultName =
            ofGetTimestampString("%Y-%m-%d_%H-%M-%S.json");
        ofFileDialogResult result =
            ofSystemSaveDialog(defaultName, "Save File", ConfigService::getService()->nottawaFolderFilePath());
        if (result.bSuccess)
        {
          ConfigService::getService()->saveConfigFile(result.getPath());
        }
      }
      if (ImGui::MenuItem("Load Config"))
      {
        // Present a file dialog to load the config file
        ofFileDialogResult result = ofSystemLoadDialog("Open File", false);
        if (result.bSuccess)
        {
          ConfigService::getService()->loadConfigFile(result.getPath());
        }
      }
      if (ImGui::MenuItem("Reset Stage"))
      {
        ShaderChainerService::getService()->clear();
        VideoSourceService::getService()->clear();
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

    if (ImGui::MenuItem("Save Default Config"))
    {
      ConfigService::getService()->saveDefaultConfigFile();
    }
    if (ImGui::MenuItem("Load Default Config"))
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

    //    static bool showingMenu = false;
    //    if (ImGui::MenuItem("ImGui Demo") || showingMenu)
    //    {
    //      ImGui::ShowDemoWindow();
    //      showingMenu = true;
    //    }

    ImGui::EndMenuBar();
  }
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
}
