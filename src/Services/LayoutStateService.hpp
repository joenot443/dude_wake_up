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

static const std::string PortraitJsonKey = "portrait";
static const std::string ShowAudioSettingsJsonKey = "showAudioSettings";
static const std::string OutputWindowUpdatesAutomaticallyJsonKey = "OutputWindowUpdatesAutomatically";
static const std::string LibraryPathJsonKey = "libraryPath";
static const std::string ColorHistoryJsonKey = "colorHistory";
static const std::string MidiEnabledJsonKey = "midiEnabled";
static const std::string StageModeEnabled = "stageModeEnabled";
static const std::string ResolutionJsonKey = "resolution";
static const std::string ShaderInfoEnabledJsonKey = "shaderInfoEnabled";
static const std::string AllParametersInStageMode = "allParametersInStageMode";
static const std::string HelpEnabledJsonKey = "helpEnabled";
static const std::string WelcomeScreenEnabledJsonKey = "welcomeScreenEnabled";
static const std::string AbletonLinkEnabledJsonKey = "abletonBPMEnabled";

static const std::vector<std::string> ResolutionOptions = {"240p", "360p", "480p", "720p", "1080p", "1440p", "4k"};
static const float MenuBarHeight = 50.0f;
static const float TabBarHeight = 15.0f;

class LayoutStateService: public ConfigurableService {
public:
  LayoutStateService(){};

  bool showAudioSettings = false;
  
  bool midiEnabled = false;
  
  bool stageModeEnabled = false;
  
  bool shaderInfoEnabled = true;
  
  bool allParametersInStageModeEnabled = true;
  
  bool helpEnabled = true;
  
  bool welcomeScreenEnabled = false;
  
  bool portrait = false;
  
  bool abletonLinkEnabled = false;
  
  bool outputWindowUpdatesAutomatically = true;
  
  int resolutionSetting = 3;
  
  int utilityPanelTab = 0;
  
  ImVec2 resolution = ImVec2(1280.0, 720.0);
  observable::subject<void()> resolutionUpdateSubject;

  // Ephemeral
  
  bool isEyeDroppingColor = false;
  
  bool isEditingText = false;
  
  bool showWelcomeScreen = true;
  
  std::string libraryPath = ConfigService::getService()->nottawaFolderFilePath();
  std::vector<std::array<float, 4>> colorHistory = std::vector<std::array<float, 4>>(1, std::array<float, 4>{0.0f, 0.0f, 0.0f, 0.0f});
  
  float audioSettingsViewHeight();
  
  ImVec2 previewSize(float scale);
  
  ImVec2 nodeLayoutSize();
  
  ImVec2 browserSize();
  
  ofRectangle canvasRect();
  
  void updateLibraryPath(std::string path);
  
  void pushColor(std::shared_ptr<std::array<float, 4>> color);
  
  bool shouldDrawShaderInfo();
  
  void updateResolutionSettings(int index);
  
  void togglePortraitSetting();
  
  void subscribeToResolutionUpdates(std::function<void()> callback);
  
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
