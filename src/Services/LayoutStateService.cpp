//
//  LayoutStateService.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 1/30/23.
//

#include "imgui.h"
#include "LayoutStateService.hpp"
#include "NodeLayoutView.hpp"
#include "ShaderChainerService.hpp"
#include "AudioSourceService.hpp"
#include "ConfigService.hpp"
#include "CommonViews.hpp"

json LayoutStateService::config() {
  json j;
  j[ShowAudioSettingsJsonKey] = showAudioSettings;
  j[LibraryPathJsonKey] = libraryPath;
  j[ColorHistoryJsonKey] = colorHistory;
  j[MidiEnabledJsonKey] = midiEnabled;
  j[StageModeEnabled] = stageModeEnabled;
  j[PortraitJsonKey] = portrait;
  j[ShaderInfoEnabledJsonKey] = shaderInfoEnabled;
  j[AllParametersInStageMode] = allParametersInStageModeEnabled;
  j[HelpEnabledJsonKey] = helpEnabled;
  j[WelcomeScreenEnabledJsonKey] = welcomeScreenEnabled;
  j[ResolutionJsonKey] = resolutionSetting;
  j[AbletonLinkEnabledJsonKey] = abletonLinkEnabled;
  j[OutputWindowUpdatesAutomaticallyJsonKey] = outputWindowUpdatesAutomatically;
  return j;
}

void LayoutStateService::loadConfig(json j) {
  showAudioSettings = j[ShowAudioSettingsJsonKey];
  libraryPath = j[LibraryPathJsonKey];
  midiEnabled = j[MidiEnabledJsonKey];
//  stageModeEnabled = j[StageModeEnabled];
  shaderInfoEnabled = j[ShaderInfoEnabledJsonKey];
  portrait = j[PortraitJsonKey];
  outputWindowUpdatesAutomatically = j[OutputWindowUpdatesAutomaticallyJsonKey];
  allParametersInStageModeEnabled = j[AllParametersInStageMode];
  helpEnabled = j[HelpEnabledJsonKey];
  welcomeScreenEnabled = j[WelcomeScreenEnabledJsonKey];
  std::vector<std::array<float, 4>> colors = j[ColorHistoryJsonKey];
  colorHistory = colors;
  showWelcomeScreen = j[WelcomeScreenEnabledJsonKey];
  resolutionSetting = j[ResolutionJsonKey];
  abletonLinkEnabled = j[AbletonLinkEnabledJsonKey];
}

void LayoutStateService::updateLibraryPath(std::string path) {
  libraryPath = path;
  ConfigService::getService()->saveDefaultConfigFile();
}

void LayoutStateService::pushColor(std::shared_ptr<std::array<float, 4>> color) {
  colorHistory.push_back(*color);
}

float LayoutStateService::audioSettingsViewHeight() {
  /*
   Three states:
      hidden: ImGuiWindowTitleBarHeight
      stopped: 40.0
      running: 250.0
   */
  if (!showAudioSettings) {
    return 0.0;
  }
  float scale = static_cast<float>(dynamic_cast<ofAppGLFWWindow*>(ofGetWindowPtr())->getPixelScreenCoordScale());
  return AudioSourceService::getService()->selectedAudioSource->active ? 250.0 : 50.0;
}

ofRectangle LayoutStateService::canvasRect() {
  return ofRectangle(LayoutStateService::getService()->browserSize().x, 0, getScaledWindowWidth() * (4/5), getScaledWindowHeight() - audioSettingsViewHeight());
}

bool LayoutStateService::shouldDrawShaderInfo() {
  return ShaderChainerService::getService()->selectedConnectable != nullptr && shaderInfoEnabled && !stageModeEnabled;
}

void LayoutStateService::updateResolutionSettings(int i) {
  resolutionSetting = i;
  if (i == 0) {
    resolution = ImVec2(426.0f, 240.0f);
  }
  else if (i == 1) {
    resolution = ImVec2(640.0f, 360.0f);
  }
  else if (i == 2) {
    resolution = ImVec2(854.0f, 480.0f);
  }
  else if (i == 3) {
    resolution = ImVec2(1280.0f, 720.0f);
  }
  else if (i == 4) {
    resolution = ImVec2(1920.0f, 1080.0f);
  }
  else if (i == 5) {
    resolution = ImVec2(2560.0f, 1440.0f);
  }
  else if (i == 6) {
    resolution = ImVec2(3840.0f, 2160.0f);
  }
  resolutionUpdateSubject.notify();
}

void LayoutStateService::togglePortraitSetting() {
  resolutionUpdateSubject.notify();
}

void LayoutStateService::subscribeToResolutionUpdates(std::function<void ()> callback) {
  resolutionUpdateSubject.subscribe(callback);
}

ImVec2 LayoutStateService::nodeLayoutSize() {
  return ImVec2(shouldDrawShaderInfo() ?  getScaledWindowWidth() - browserSize().x*2 :
                getScaledWindowWidth() - browserSize().x,
                getScaledWindowHeight() - audioSettingsViewHeight());
}

ImVec2 LayoutStateService::browserSize() {
  float width = fmax(getScaledWindowWidth() / 5., 321.0);
  return ImVec2(width, (ImGui::GetWindowContentRegionMax().y - MenuBarHeight) / 3.);
}

ImVec2 LayoutStateService::previewSize(float scale) {
  bool portrait = LayoutStateService::getService()->portrait;
  return portrait ? ImVec2(500 * 4.0 / scale, 281.25 * 4.0 /scale) : ImVec2(500 * 4.0 /scale, 281.25 * 4.0 /scale);
}
