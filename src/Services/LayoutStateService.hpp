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

class LayoutStateService: public ConfigurableService {
public:
  LayoutStateService(){};

  bool showAudioSettings = false;
  std::string libraryPath = ConfigService::getService()->nottawaFolderFilePath();
  
  void updateLibraryPath(std::string path);
  
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
