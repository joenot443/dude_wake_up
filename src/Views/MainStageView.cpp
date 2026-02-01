//
//  MainStageView.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 11/14/22.
//

//#include <sentry.h>
#include "MainStageView.hpp"
#include <CoreFoundation/CoreFoundation.h>
#include "ShaderInfoView.hpp"
#include "ActionService.hpp"
#include "CommonStrings.hpp"
#include "CommonViews.hpp"
#include "ConfigService.hpp"
#include "TileBrowserView.hpp"
#include "ParameterTileBrowserView.hpp"
#include "MarkdownView.hpp"
#include "AudioSourceService.hpp"
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
#include "VideoSourceService.hpp"
#include "../Services/SparkleUpdater.h"
#include "implot.h"
#include "FramePreview.hpp"

static const ImVec2 ShaderButtonSize = ImVec2(90, 30);

void MainStageView::setup()
{
  NodeLayoutView::getInstance()->setup();
  shaderBrowserView.setup();
  strandBrowserView.setup();
  videoSourceBrowserView.setup();
  audioSourceBrowserView.setup();
  videoSourceBrowserView.setCurrentTab(0);
  shaderBrowserView.setCurrentTab(0);
  stageModeView.setup();
  welcomeScreenView.setup();

  // Wire up collapse state pointers
  videoSourceBrowserView.collapsed = &sourceBrowserCollapsed;
  shaderBrowserView.collapsed = &shaderBrowserCollapsed;
}

void MainStageView::update()
{
  videoSourceBrowserView.update();
  audioSourceBrowserView.update();
  shaderBrowserView.update();
  strandBrowserView.update();
  if (LayoutStateService::getService()->stageModeEnabled) {
    stageModeView.update();
  }
  NodeLayoutView::getInstance()->update();
  if (LayoutStateService::getService()->showWelcomeScreen) {
    welcomeScreenView.update();
  }
  if (showShareStrandWindow) {
    shareStrandView.update();
  }

  if (drawFPS) {
    ofSetWindowTitle(formatString("%.2f FPS", ofGetFrameRate()));
  }
}

void MainStageView::draw()
{
  // Draw a table with 2 or columns, sized | 1/5 |   4/5   | 1/5 |
  ImVec2 nodeLayoutSize = LayoutStateService::getService()->nodeLayoutSize();
  
  ImVec2 browserSize = LayoutStateService::getService()->browserSize();
  
  //  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(15.0, 15.0));
  float leftColumnWidth = leftPanelCollapsed ? 20.0f : browserSize.x;
  int columnCount = LayoutStateService::getService()->shouldDrawShaderInfo() ? 3 : 2;
  ImGui::Columns(columnCount, "main_stage_view", false);
  ImGui::SetColumnWidth(0, leftColumnWidth);
  ImGui::SetColumnWidth(1, nodeLayoutSize.x + (leftPanelCollapsed ? browserSize.x - 20.0f : 0.0f));
  if (LayoutStateService::getService()->shouldDrawShaderInfo()) {
    ImGui::SetColumnWidth(2, browserSize.x);
  }
  
  
  // | Sources |  Node Layout
  // | Effects |        ''
  // | Library |       Audio

  if (leftPanelCollapsed) {
    float panelHeight = browserSize.y * 3;
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.2f, 0.2f, 0.5f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.4f, 0.4f, 0.4f, 0.7f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.5f, 0.5f, 0.5f, 0.8f));
    if (ImGui::Button(">>##expandPanel", ImVec2(10.0f, panelHeight))) {
      leftPanelCollapsed = false;
    }
    ImGui::PopStyleColor(3);
  }

  if (!leftPanelCollapsed) {
  // Calculate dynamic heights based on collapse states
  const float collapsedHeight = 56.0f; // Height when collapsed (just shows the search bar/header)
  const float totalHeight = browserSize.y * 3; // Total available height for all three sections

  // Count how many sections are expanded
  int expandedCount = 0;
  if (!sourceBrowserCollapsed) expandedCount++;
  if (!shaderBrowserCollapsed) expandedCount++;
  if (!utilityPanelCollapsed) expandedCount++;

  // Calculate available height for expanded sections
  float usedByCollapsed = 0;
  if (sourceBrowserCollapsed) usedByCollapsed += collapsedHeight;
  if (shaderBrowserCollapsed) usedByCollapsed += collapsedHeight;
  if (utilityPanelCollapsed) usedByCollapsed += collapsedHeight;

  float availableForExpanded = totalHeight - usedByCollapsed;
  float expandedSectionHeight = expandedCount > 0 ? availableForExpanded / expandedCount : browserSize.y;

  // Calculate individual section heights with margin for rounded corners
  const float collapseBarWidth = 10.0f;
  const float sideMargin = 18.0f;
  ImVec2 collapseBarStartPos = ImGui::GetCursorPos();
  ImVec2 sourceBrowserSize = ImVec2(browserSize.x - sideMargin, sourceBrowserCollapsed ? collapsedHeight : expandedSectionHeight);
  ImVec2 shaderBrowserSize = ImVec2(browserSize.x - sideMargin, shaderBrowserCollapsed ? collapsedHeight : expandedSectionHeight);
  ImVec2 utilityBrowserSize = ImVec2(browserSize.x - sideMargin, utilityPanelCollapsed ? collapsedHeight : expandedSectionHeight);

  // Sources
  ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 6.0f);
  ImGui::PushStyleColor(ImGuiCol_ChildBg, Colors::TabSelected.Value);
  ImGui::BeginChild("##sourceBrowser", sourceBrowserSize, ImGuiChildFlags_Border, ImGuiWindowFlags_AlwaysUseWindowPadding | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
  ImGui::PopStyleColor();
  drawVideoSourceBrowser();
  ImGui::EndChild();
  ImGui::PopStyleVar();

  ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 6.0f);
  ImGui::PushStyleColor(ImGuiCol_ChildBg, Colors::TabSelected.Value);
  ImGui::BeginChild("##shaderBrowser", shaderBrowserSize, ImGuiChildFlags_Border, ImGuiWindowFlags_AlwaysUseWindowPadding | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
  ImGui::PopStyleColor();
  drawShaderBrowser();
  ImGui::EndChild();
  ImGui::PopStyleVar();

  if (ImGui::GetContentRegionAvail().x < 10) { return; }


  ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 6.0f);
  ImGui::PushStyleColor(ImGuiCol_ChildBg, Colors::TabSelected.Value);
  if (ImGui::BeginChild("##libraryOscillatorBrowser", utilityBrowserSize, ImGuiChildFlags_Border, ImGuiWindowFlags_AlwaysUseWindowPadding | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse)) {
  ImGui::PopStyleColor();

    // Draw collapse button before the tab bar
    CommonViews::CollapsibleSearchHeader(
      "Utility",
      "Search Strands",
      &utilityPanelCollapsed,
      LayoutStateService::getService()->strandSearchQuery,
      LayoutStateService::getService()->strandSearchDirty,
      "StrandSearchClear"
    );

    // Only draw tabs and content if not collapsed
    if (!utilityPanelCollapsed) {
      if (ImGui::BeginTabBar("##LibraryOscillatorTabBar")) {

        // My Strands Tab
        if (ImGui::BeginTabItem("My Strands", 0, LayoutStateService::getService()->utilityPanelTab == 0 ? ImGuiTabItemFlags_SetSelected : ImGuiTabItemFlags_None)) {
          // Set the tab if user clicks manually
          strandBrowserView.draw();
          ImGui::EndTabItem();
        }
        if (ImGui::IsItemClicked()) {
          LayoutStateService::getService()->utilityPanelTab = 0;
        }

        // Oscillators Tab
        if (ImGui::BeginTabItem("Oscillators", 0, LayoutStateService::getService()->utilityPanelTab == 1 ? ImGuiTabItemFlags_SetSelected : ImGuiTabItemFlags_None)) {
          // Set the tab if user clicks manually
          oscillatorPanelView.draw();

          ImGui::EndTabItem();
        }
        if (ImGui::IsItemClicked()) {
          LayoutStateService::getService()->utilityPanelTab = 1;
        }

        if (ImGui::BeginTabItem("Time", 0, LayoutStateService::getService()->utilityPanelTab == 2 ? ImGuiTabItemFlags_SetSelected : ImGuiTabItemFlags_None)) {
          timePanelView.draw();
          ImGui::EndTabItem();
        }
        if (ImGui::IsItemClicked()) {
          LayoutStateService::getService()->utilityPanelTab = 2;
        }

        ImGui::EndTabBar();
      }
    }
  }

  ImGui::EndChild();
  ImGui::PopStyleVar(); // Pop ChildRounding

  // Now, handle external tab selection by checking `utilityPanelTab`
  if (LayoutStateService::getService()->utilityPanelTab == 0) {
    ImGui::SetTabItemClosed("Oscillators");  // Reflect external control by closing the other tab
  } else if (LayoutStateService::getService()->utilityPanelTab == 1) {
    ImGui::SetTabItemClosed("My Strands");   // Reflect external control by closing the other tab
  }
  
  // Collapse left panel button — narrow vertical bar to the right of browsers
  float collapseBarHeight = totalHeight;
  ImGui::SetCursorPos(ImVec2(browserSize.x - sideMargin, collapseBarStartPos.y));
  ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.2f, 0.2f, 0.5f));
  ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.4f, 0.4f, 0.4f, 0.7f));
  ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.5f, 0.5f, 0.5f, 0.8f));
  ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 3.0f);
  if (ImGui::Button("##collapsePanel", ImVec2(sideMargin, collapseBarHeight))) {
    leftPanelCollapsed = true;
  }
  ImGui::PopStyleVar();
  ImGui::PopStyleColor(3);
  } // end if (!leftPanelCollapsed)

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
    auto shaderInfoPaneSize = ImVec2(browserSize.x ,(ImGui::GetWindowContentRegionMax().y - MenuBarHeight));
    
    ImGui::NextColumn();
    ImGui::BeginChild("##shaderInfo", shaderInfoPaneSize, false, ImGuiWindowFlags_AlwaysUseWindowPadding);
    ShaderInfoView::draw();
    ImGui::EndChild();
  }
  
  // Welcome Screen
  
    if (LayoutStateService::getService()->showWelcomeScreen) {
      welcomeScreenView.draw();
    }

  if (showFeedbackWindow) {
    feedbackView.draw(&showFeedbackWindow);
  }

  // Check if NodeLayoutView wants to share a strand
  if (NodeLayoutView::getInstance()->hasPendingShareStrand) {
    NodeLayoutView::getInstance()->hasPendingShareStrand = false;
    shareStrandView.show(NodeLayoutView::getInstance()->pendingShareStrand);
    showShareStrandWindow = true;
  }

  if (showShareStrandWindow) {
    shareStrandView.draw(&showShareStrandWindow);
  }
}

void MainStageView::drawMenu()
{
  if (ImGui::BeginMenuBar())
  {
    /// #File Menu
    if (ImGui::BeginMenu("File"))
    {
      if (ImGui::MenuItem("Save", "Cmd+S", false, ConfigService::getService()->isEditingWorkspace())) {
        ConfigService::getService()->saveCurrentWorkspace();
      }
      
      if (ImGui::MenuItem("Save Workspace As", "Cmd+Shift+S"))
      {
        ConfigService::getService()->saveNewWorkspace();
      }

      ImGui::Separator();

      if (ImGui::MenuItem("Share Strand...", ""))
      {
        // Build a strand from all current connectables
        std::vector<std::shared_ptr<Connectable>> connectables;
        for (auto& shader : ShaderChainerService::getService()->shaders()) {
          connectables.push_back(shader);
        }
        for (auto& source : VideoSourceService::getService()->videoSources()) {
          connectables.push_back(source);
        }

        if (!connectables.empty()) {
          Strand strand;
          strand.connectables = connectables;
          strand.connections = ShaderChainerService::getService()->connections();
          strand.name = Strand::strandName(connectables);

          shareStrandView.show(strand);
          showShareStrandWindow = true;
        }
      }

      ImGui::Separator();

      if (ImGui::MenuItem("Open Workspace", "Cmd+O"))
      {
        ConfigService::getService()->loadWorkspaceDialogue();
      }
      if (ImGui::MenuItem("New Workspace", "Cmd+N"))
      {
        ShaderChainerService::getService()->clear();
        VideoSourceService::getService()->clear();
        ConfigService::getService()->closeWorkspace();
      }
      #ifdef TARGET_OSX
      if (ImGui::MenuItem("Check for Updates..."))
      {
        SparkleUpdaterService::getInstance().checkForUpdates();
      }
      #endif
      ImGui::EndMenu();
    }
    
    /// #Edit Menu

    if (ImGui::BeginMenu("Edit"))
    {
      if (ImGui::MenuItem("Undo", "Cmd+Z"))
      {
        ActionService::getService()->undo();
      }

      if (ImGui::MenuItem("Redo", "Cmd+Y"))
      {
        ActionService::getService()->redo();
      }

      ImGui::Separator();

      if (ImGui::MenuItem("Clear Feedback Buffers", "Cmd+K"))
      {
        FeedbackSourceService::getService()->clearBuffers();
      }
      ImGui::EndMenu();
    }

    /// #Capture Menu

    if (ImGui::BeginMenu("Capture"))
    {
      if (ImGui::MenuItem("Capture Output Window"))
      {
        VideoSourceService::getService()->captureOutputWindowScreenshot();
      }

      if (ImGui::MenuItem("Capture All Nodes"))
      {
        VideoSourceService::getService()->captureAllNodesScreenshots();
      }
      ImGui::EndMenu();
    }

    /// #Output Menu

    if (ImGui::BeginMenu("Display"))
    {
      bool enabled = LayoutStateService::getService()->outputWindowUpdatesAutomatically;
      std::string title = CommonStrings::EnableOutputWindowUpdatesAutomatically;
      
      if (ImGui::MenuItem(title.c_str(), nullptr, &enabled))
      {
        LayoutStateService::getService()->outputWindowUpdatesAutomatically = enabled;
      }
      
      if (ImGui::BeginMenu("Resolution"))
      {
        std::vector<std::string> resolutions = ResolutionOptions;
        int i = 0;
        for (const std::string& option : resolutions) {
          bool selected = LayoutStateService::getService()->resolutionSetting == i;
          if (ImGui::MenuItem(option.c_str(), "", &selected)) {
            LayoutStateService::getService()->updateResolutionSettings(i);
          }
          i++;
        }
        ImGui::EndMenu();
      }
      
      if (ImGui::MenuItem("Portrait Mode", nullptr, &LayoutStateService::getService()->portrait))
      {
        LayoutStateService::getService()->togglePortraitSetting();
      }
      
      ImGui::EndMenu();
    }

    /// #Audio Menu
    if (ImGui::BeginMenu("Audio"))
    {
      bool analysisEnabled = AudioSourceService::getService()->selectedAudioSource->active;
      if (ImGui::MenuItem(analysisEnabled ? "Stop Analysis" : "Start Analysis"))
      {
        AudioSourceService::getService()->selectedAudioSource->active = !analysisEnabled;
      }

      if (ImGui::BeginMenu("Audio Source"))
      {
        auto audioSources = AudioSourceService::getService()->audioSources();
        auto selectedSource = AudioSourceService::getService()->selectedAudioSource;
        
        for (auto& source : audioSources) {
          bool isSelected = (source == selectedSource);
          if (ImGui::MenuItem(source->name.c_str(), nullptr, isSelected)) {
            AudioSourceService::getService()->selectedAudioSource = source;
            AudioSourceService::getService()->selectedAudioSource->active = true;
          }
        }
        ImGui::EndMenu();
      }

      bool linkEnabled = LayoutStateService::getService()->abletonLinkEnabled;
      if (ImGui::MenuItem("Enable Ableton Link", nullptr, &linkEnabled))
      {
        LayoutStateService::getService()->abletonLinkEnabled = linkEnabled;
      }

      ImGui::EndMenu();
    }
    
    /// #View Menu
    
    if (ImGui::BeginMenu("View"))
    {
      bool midiEnabled = LayoutStateService::getService()->midiEnabled;
      std::string title = CommonStrings::EnableMidi;
      
      if (ImGui::MenuItem(title.c_str(), nullptr, &midiEnabled))
      {
        LayoutStateService::getService()->midiEnabled = midiEnabled;
      }
      
      bool flowEnabled = LayoutStateService::getService()->flowEnabled;
      if (ImGui::MenuItem("Animate Connection Flow", nullptr, &flowEnabled))
      {
        LayoutStateService::getService()->flowEnabled = flowEnabled;
      }
      
//      if (ImGui::MenuItem("Toggle Stage Mode", "Cmd+B"))
//      {
//        LayoutStateService::getService()->stageModeEnabled = !LayoutStateService::getService()->stageModeEnabled;
//      }
      if (ImGui::MenuItem("Launch Welcome Screen"))
      {
        LayoutStateService::getService()->showWelcomeScreen = true;
      }
      ImGui::EndMenu();
    }
    

    if (ImGui::MenuItem("Send Feedback"))
    {
      showFeedbackWindow = true;
    }

    static bool showingMenu = false;
#ifndef RELEASE
    if (ImGui::MenuItem("ImGui Demo") || showingMenu)
    {
      ImGui::ShowDemoWindow();
      showingMenu = true;
    }
    if (ImGui::MenuItem("Save Default Workspace"))
    {
      ConfigService::getService()->saveDefaultConfigFile();
    }
    if (ImGui::MenuItem("Load Default Workspace"))
    {
      ConfigService::getService()->loadDefaultConfigFile();
    }
#endif
    ImGui::EndMenuBar();
  }
}

void MainStageView::drawVideoSourceBrowser()
{
  videoSourceBrowserView.draw();
}

void MainStageView::drawShaderBrowser() { shaderBrowserView.draw(); }

void MainStageView::loadDirectory(std::string directory)
{
  videoSourceBrowserView.loadDirectory(directory);
}

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
    
    if (key == 'z') {
      ActionService::getService()->undo();
    }
    
    if (key == 'y') {
      ActionService::getService()->redo();
    }
    
    if (key == 'c') {
      ActionService::getService()->copy(NodeLayoutView::getInstance()->selectedConnectables);
    }
    
    if (key == 'v') {
      ActionService::getService()->paste();
    }
  }
  
  if (key == OF_KEY_SHIFT) {
    drawFPS = false;
    ofSetWindowTitle("");
  }
  
  if (key == OF_KEY_ESC) {
    LayoutStateService::getService()->showWelcomeScreen = false;
  }
}


void MainStageView::keyPressed(int key) {
  if (key == OF_KEY_SHIFT) {
    drawFPS = true;
  }
}
