#include <errno.h>
#include <sentry.h>

#include "imgui.h"
#include "CommonViews.hpp"
#include "ofMain.h"
#include "MainApp.h"
#include "FontService.hpp"
#include "ModulationService.hpp"
#include "OscillationService.hpp"
#include "AudioSourceService.hpp"
#include "MidiService.hpp"
#include "ConfigService.hpp"
#include "FeedbackSourceService.hpp"
#include "LibraryService.hpp"
#include "VideoSourceService.hpp"
#include "NodeLayoutView.hpp"
#include "MarkdownService.hpp"
#include "TextureService.hpp"
#include "ShaderChainerService.hpp"
#include "LayoutStateService.hpp"
#include "HelpService.hpp"
#include "StrandService.hpp"
#include "ParameterService.hpp"
#include "IconService.hpp"

const static ofVec2f windowSize = ofVec2f(2400, 1600);

using json = nlohmann::json;

#define RELEASE

const char* SubmitFeedbackView::popupId = "Submit Feedback";

NodeLayoutView *NodeLayoutView::instance = 0;
StrandService *StrandService::service = 0;
FontService *FontService::service = 0;
TextureService *TextureService::service = 0;
IconService *IconService::service = 0;
ModulationService *ModulationService::service = 0;
OscillationService *OscillationService::service = 0;
MidiService *MidiService::service = 0;
ParameterService *ParameterService::service = 0;
ConfigService *ConfigService::service = 0;
FeedbackSourceService *FeedbackSourceService::service = 0;
VideoSourceService *VideoSourceService::service = 0;
ShaderChainerService *ShaderChainerService::service = 0;
AudioSourceService *AudioSourceService::service = 0;
LayoutStateService *LayoutStateService::service = 0;
MarkdownService *MarkdownService::service = 0;
LibraryService *LibraryService::service = 0;
HelpService *HelpService::service = 0;
MainApp *MainApp::app = 0;

void setupDirectories() {
  ofSetDataPathRoot("../Resources/data/");
  auto homeDir = ofFilePath::getUserHomeDir();
  auto libraryPath = ofFilePath::join(homeDir, "/nottawa");
  auto sentryPath = ofFilePath::join(homeDir, "/nottawa/sentry");
  auto shadersPath = ofFilePath::join(libraryPath, "/shaders");
  auto videosPath = ofFilePath::join(libraryPath, "/videos");
  
  if (!ofDirectory::doesDirectoryExist(libraryPath)) {
    ofDirectory::createDirectory(libraryPath);
  }
  if (!ofDirectory::doesDirectoryExist(sentryPath)) {
    ofDirectory::createDirectory(sentryPath);
  }
  if (!ofDirectory::doesDirectoryExist(shadersPath)) {
    ofDirectory::createDirectory(shadersPath);
  }
  if (!ofDirectory::doesDirectoryExist(videosPath)) {
    ofDirectory::createDirectory(videosPath);
  }
  
  for (auto dir : ConfigService::getService()->shaderConfigFoldersPaths()) {
    if (!ofDirectory::doesDirectoryExist(dir)) {
      ofDirectory::createDirectory(dir);
    }
  }
}

int main( ){
  setupDirectories();
  auto homeDir = ofFilePath::getUserHomeDir();
  auto libraryPath = ofFilePath::join(homeDir, "/nottawa");
  auto sentryPath = ofFilePath::join(homeDir, "/nottawa/sentry");
  
  sentry_options_t *options = sentry_options_new();
  sentry_options_set_dsn(options, "https://ce0071f5f4c84c6f99f58a6a7d97c5a1@o4505431303847936.ingest.sentry.io/4505431305486336");
  sentry_options_set_database_path(options, sentryPath.c_str());
  auto crashpadDirectory = ofFilePath::join(ofFilePath::getCurrentExeDir(), "../../../crashpad_handler");
  log(crashpadDirectory);
  sentry_options_set_handler_path(options, crashpadDirectory.c_str());
  sentry_options_set_release(options, "nottawa@0.1");
  sentry_options_set_debug(options, 1);
//  sentry_init(options);

  ofGLFWWindowSettings settings;
  settings.setSize(1920, 1080);
  settings.setGLVersion(3, 2);
  auto window = ofCreateWindow(settings);
  auto app = shared_ptr<MainApp>(new MainApp(window));
  ofSetWindowShape(ofGetScreenWidth(), ofGetScreenHeight());
  ofSetFrameRate(60);
  ofSetEscapeQuitsApp(false);
  ofRunApp(window, app);
  ofRunMainLoop();
  sentry_close();
}



