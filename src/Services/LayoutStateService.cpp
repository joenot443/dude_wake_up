//
//  LayoutStateService.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 1/30/23.
//

#include "imgui.h"
#include "LayoutStateService.hpp"
#include "ShaderChainerService.hpp"
#include "AudioSourceService.hpp"
#include "ConfigService.hpp"

json LayoutStateService::config() {
  json j;
  j[ShowAudioSettingsJsonKey] = showAudioSettings;
  j[LibraryPathJsonKey] = libraryPath;
  j[ColorHistoryJsonKey] = colorHistory;
  j[MidiEnabledJsonKey] = midiEnabled;
  j[StageModeEnabled] = stageModeEnabled;
  j[ShaderInfoEnabledJsonKey] = shaderInfoEnabled;
  j[AllParametersInStageMode] = allParametersInStageModeEnabled;
  j[HelpEnabledJsonKey] = helpEnabled;
  return j;
}

void LayoutStateService::loadConfig(json j) {
  showAudioSettings = j[ShowAudioSettingsJsonKey];
  libraryPath = j[LibraryPathJsonKey];
  midiEnabled = j[MidiEnabledJsonKey];
  stageModeEnabled = j[StageModeEnabled];
  shaderInfoEnabled = j[ShaderInfoEnabledJsonKey];
  allParametersInStageModeEnabled = j[AllParametersInStageMode];
  helpEnabled = j[HelpEnabledJsonKey];
  std::vector<std::array<float, 4>> colors = j[ColorHistoryJsonKey];
  colorHistory = colors;
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
  return AudioSourceService::getService()->selectedAudioSource->active ? 250.0 : 40.0;
}

ofRectangle LayoutStateService::canvasRect() {
  return ofRectangle(getScaledWindowWidth() / 5, 0, getScaledWindowWidth() * (4/5), getScaledWindowHeight() - audioSettingsViewHeight());
}

bool LayoutStateService::shouldDrawShaderInfo() {
  return ShaderChainerService::getService()->selectedConnectable != nullptr && shaderInfoEnabled && !stageModeEnabled;
}
