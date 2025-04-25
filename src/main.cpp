#define JSON_DIAGNOSTICS 1
#define IMGUI_DEFINE_MATH_OPERATORS 1
#define DEBUG 1
#define RELEASE 1

#include <errno.h>
#include <CoreFoundation/CoreFoundation.h>

#include "imgui.h"
#include "imgui_internal.h"
#include "CommonViews.hpp"
#include "ofMain.h"
#include "MainApp.h"
#include "FontService.hpp"
#include "ModulationService.hpp"
#include "OscillationService.hpp"
#include "AudioSourceService.hpp"
#include "MidiService.hpp"
#include "ConfigService.hpp"
#include "ImageService.hpp"
#include "FeedbackSourceService.hpp"
#include "LibraryService.hpp"
#include "VideoSourceService.hpp"
#include "NodeLayoutView.hpp"
#include "MarkdownService.hpp"
#include "TextureService.hpp"
#include "ShaderChainerService.hpp"
#include "LayoutStateService.hpp"
#include "HelpService.hpp"
#include "BookmarkService.hpp"
#include "StrandService.hpp"
#include "ParameterService.hpp"
#include "IconService.hpp"
#include "ofxFastFboReader.h"


const static ofVec2f windowSize = ofVec2f(2400, 1600);

using json = nlohmann::json;

const char* SubmitFeedbackView::popupId = "Submit Feedback";

NodeLayoutView *NodeLayoutView::instance = 0;
BookmarkService *BookmarkService::service = 0;
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
ImageService *ImageService::service = 0;
MarkdownService *MarkdownService::service = 0;
LibraryService *LibraryService::service = 0;
HelpService *HelpService::service = 0;
//SyphonService *SyphonService::service = 0;
MainApp *MainApp::app = 0;

void setupDirectories() {
  ofSetDataPathRoot("../Resources/data/");
  
  auto appSupportPath = ConfigService::appSupportFilePath();
  auto libraryPath = appSupportPath + "/nottawa";
  auto sentryPath = libraryPath + "/sentry";
  auto shadersPath = libraryPath + "/shaders";
  auto videosPath = libraryPath + "/videos";
  auto strandsPath = libraryPath + "/strands";
  auto exportPath = libraryPath + "/exports";
  
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
  if (!ofDirectory::doesDirectoryExist(strandsPath)) {
    ofDirectory::createDirectory(strandsPath);
  }
  if (!ofDirectory::doesDirectoryExist(exportPath)) {
    ofDirectory::createDirectory(exportPath);
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
  
  ofGLFWWindowSettings settings;
  settings.setSize(1920, 1080);
  settings.setGLVersion(3, 2);
  auto window = ofCreateWindow(settings);
  auto app = shared_ptr<MainApp>(new MainApp(window));
  ofSetWindowShape(ofGetScreenWidth(), ofGetScreenHeight());
  ofSetFrameRate(60);
  ofSetEscapeQuitsApp(false);
  ofSetWindowTitle("Nottawa");
  ofSetLogLevel(OF_LOG_NOTICE);
  ofRunApp(window, app);
  ofRunMainLoop();
  return 0;
}



