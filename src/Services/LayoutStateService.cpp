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
  return j;
}

void LayoutStateService::loadConfig(json j) {
  showAudioSettings = j[ShowAudioSettingsJsonKey];
  libraryPath = j[LibraryPathJsonKey];
}

void LayoutStateService::updateLibraryPath(std::string path) {
  libraryPath = path;
  ConfigService::getService()->saveDefaultConfigFile();
}
