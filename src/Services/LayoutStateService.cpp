//
//  LayoutStateService.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 1/30/23.
//

#include "LayoutStateService.hpp"
#include "ConfigService.hpp"

json LayoutStateService::config() {
  json j;
  j[ShowAudioSettingsJsonKey] = showAudioSettings;
  j[LibraryPathJsonKey] = libraryPath;
  j[ColorHistoryJsonKey] = colorHistory;
  return j;
}

void LayoutStateService::loadConfig(json j) {
  showAudioSettings = j[ShowAudioSettingsJsonKey];
  libraryPath = j[LibraryPathJsonKey];
  std::vector<std::array<float, 3>> colors = j[ColorHistoryJsonKey];
  colorHistory = colors;
}

void LayoutStateService::updateLibraryPath(std::string path) {
  libraryPath = path;
  ConfigService::getService()->saveDefaultConfigFile();
}

void LayoutStateService::pushColor(std::shared_ptr<std::array<float, 3>> color) {
  colorHistory.push_back(*color);
}
