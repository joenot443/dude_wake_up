//
//  LayoutStateService.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 1/30/23.
//

#ifndef LayoutStateService_hpp
#define LayoutStateService_hpp

#include <stdio.h>
#include "ofMain.h"
#include "ConfigService.hpp"
#include "ConfigurableService.hpp"

static const std::string ShowAudioSettingsJsonKey = "showAudioSettings";
static const std::string LibraryPathJsonKey = "libraryPath";
static const std::string ColorHistoryJsonKey = "colorHistory";
static const std::string MidiEnabledJsonKey = "midiEnabled";
static const std::string StageModeEnabled = "stageModeEnabled";
static const std::string ResolutionJsonKey = "resolution";

class LayoutStateService: public ConfigurableService {
public:
  LayoutStateService(){};

  bool showAudioSettings = false;
  
  bool midiEnabled = false;
  
  bool stageModeEnabled = false;
  
  std::string libraryPath = ConfigService::getService()->nottawaFolderFilePath();
  std::vector<std::array<float, 3>> colorHistory = std::vector<std::array<float, 3>>(1, std::array<float, 3>{0.0f, 0.0f, 0.0f});
  
  float audioSettingsViewHeight();
  
  ofRectangle canvasRect();
  
  void updateLibraryPath(std::string path);
  
  void pushColor(std::shared_ptr<std::array<float, 3>> color);
  
  static LayoutStateService *service;

  static LayoutStateService *getService() {
    if (!service) {
      service = new LayoutStateService;
    }
    return service;
  }
  
  json config();
  void loadConfig(json j);
};

#endif /* LayoutStateService_hpp */
