#include <errno.h>
#include "ofMain.h"
#include "MainApp.h"
#include "FontService.hpp"
#include "ModulationService.hpp"
#include "OscillationService.hpp"
#include "AudioSourceService.hpp"
#include "MidiService.hpp"
#include "ConfigService.hpp"
#include "FeedbackSourceService.hpp"
#include "VideoSourceService.hpp"
#include "MarkdownService.hpp"
#include "ShaderChainerService.hpp"
#include "LayoutStateService.hpp"
#include "ParameterService.hpp"

using json = nlohmann::json;

FontService *FontService::service = 0;
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

void setupDirectories() {
  ofSetDataPathRoot("../Resources/data/");
  auto homeDir = ofFilePath::getUserHomeDir();
  auto libraryPath = ofFilePath::join(homeDir, "/nottawa");
  if (!ofDirectory::doesDirectoryExist(libraryPath)) {
    ofDirectory::createDirectory(libraryPath);
  }
}

int main( ){
  setupDirectories();

  ofGLFWWindowSettings settings;
  // Set the window size to be the same as the monitor size
  settings.setSize(1440, 900);
  settings.setGLVersion(3, 2);
  //  ofSetupOpenGL(1280,720, OF_WINDOW);
  //  settings.windowMode = OF_WINDOW;
  auto window = ofCreateWindow(settings);
  auto app = shared_ptr<MainApp>(new MainApp(window));
  ofSetFrameRate(60);
  ofRunApp(window, app);
  ofRunMainLoop();
}



