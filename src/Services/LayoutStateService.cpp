//
//  LayoutStateService.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 1/30/23.
//

#include "imgui.h"
#include "LayoutStateService.hpp"
#include "BookmarkService.hpp"
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
  j[OscillationEnabledJsonKey] = oscillationEnabled;
  j[StageModeEnabled] = stageModeEnabled;
  j[PortraitJsonKey] = portrait;
  j[AllParametersInStageMode] = allParametersInStageModeEnabled;
  j[HelpEnabledJsonKey] = helpEnabled;
  j[WelcomeScreenEnabledJsonKey] = welcomeScreenEnabled;
  j[ResolutionJsonKey] = resolutionSetting;
  j[AbletonLinkEnabledJsonKey] = abletonLinkEnabled;
  j[OutputWindowUpdatesAutomaticallyJsonKey] = outputWindowUpdatesAutomatically;
  j[ActionBarExpandedJsonKey] = actionBarExpanded;
  j[FlowEnabledJsonKey] = flowEnabled;
  return j;
}

void LayoutStateService::loadConfig(json j) {
  showAudioSettings = j[ShowAudioSettingsJsonKey];
  if (!j[LibraryPathJsonKey].is_null()) {
    updateLibraryPath(j[LibraryPathJsonKey]);
  }
  libraryPath = j[LibraryPathJsonKey];
  midiEnabled = j[MidiEnabledJsonKey];
  // Back-compat: key absent in configs saved before the global toggle existed.
  if (!j[OscillationEnabledJsonKey].is_null()) {
    oscillationEnabled = j[OscillationEnabledJsonKey];
  }
//  stageModeEnabled = j[StageModeEnabled];
  portrait = j[PortraitJsonKey];
  outputWindowUpdatesAutomatically = j[OutputWindowUpdatesAutomaticallyJsonKey];
  allParametersInStageModeEnabled = j[AllParametersInStageMode];
  helpEnabled = j[HelpEnabledJsonKey];
  welcomeScreenEnabled = j[WelcomeScreenEnabledJsonKey];
  std::vector<std::array<float, 4>> colors = j[ColorHistoryJsonKey];
  colorHistory = colors;
  showWelcomeScreen = j[WelcomeScreenEnabledJsonKey];
  // Go through updateResolutionSettings so the pixel-dimension ImVec2 stays
  // in sync with the int index. Writing resolutionSetting directly was a
  // long-standing bug: subsequent FBO allocations would use the still-default
  // ImVec2 (1280×720) regardless of what the user had saved, until they
  // happened to re-trigger setResolution through the Settings UI.
  updateResolutionSettings(j[ResolutionJsonKey]);
  abletonLinkEnabled = j[AbletonLinkEnabledJsonKey];
  actionBarExpanded = j[ActionBarExpandedJsonKey];
  flowEnabled = j[FlowEnabledJsonKey];
}

void LayoutStateService::updateLibraryPath(std::string path) {
  BookmarkService::getService()->endAccessingBookmark(libraryPath);
  libraryPath = path;
  BookmarkService::getService()->beginAccessingBookmark(libraryPath);
  ConfigService::getService()->saveDefaultConfigFile();
}

void LayoutStateService::pushColor(std::shared_ptr<std::array<float, 4>> color) {
  colorHistory.push_back(*color);
}

float LayoutStateService::audioSettingsViewHeight() {
  if (!AudioSourceService::getService()->selectedAudioSource->active) {
    return 30.0;
  }
  return 250.0;
}

ofRectangle LayoutStateService::canvasRect() {
  return ofRectangle(LayoutStateService::getService()->browserSize().x, 0, getScaledWindowWidth() * (4/5), getScaledWindowHeight() - audioSettingsViewHeight());
}

bool LayoutStateService::shouldDrawShaderInfo() {
  return ShaderChainerService::getService()->selectedConnectable != nullptr && !stageModeEnabled;
}

void LayoutStateService::updateResolutionSettings(int i) {
  // Single source of truth for the index → pixel-dimension table. Anything
  // changing resolution MUST go through here, never via direct writes to
  // resolutionSetting (which would leave the ImVec2 stale).
  static const ImVec2 sizes[] = {
    {  426.0f,  240.0f }, // 0 — 240p
    {  640.0f,  360.0f }, // 1 — 360p
    {  854.0f,  480.0f }, // 2 — 480p
    { 1280.0f,  720.0f }, // 3 — 720p
    { 1920.0f, 1080.0f }, // 4 — 1080p
    { 2560.0f, 1440.0f }, // 5 — 1440p
    { 3840.0f, 2160.0f }, // 6 — 4K
  };
  if (i < 0 || i >= static_cast<int>(sizeof(sizes) / sizeof(sizes[0]))) {
    return;
  }
  resolutionSetting = i;
  resolution = sizes[i];
  resolutionUpdateSubject.notify();
}

void LayoutStateService::setCustomResolution(float width, float height) {
  resolution = ImVec2(width, height);
  resolutionUpdateSubject.notify();
}

void LayoutStateService::togglePortraitSetting() {
  resolutionUpdateSubject.notify();
}

observable::unique_subscription LayoutStateService::subscribeToResolutionUpdates(std::function<void ()> callback) {
  return observable::unique_subscription(resolutionUpdateSubject.subscribe(callback));
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
