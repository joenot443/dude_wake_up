#include "ofMain.h"
#include "MainApp.h"
#include "FontService.hpp"
#include "ModulationService.hpp"
#include "OscillationService.hpp"
#include "MidiService.hpp"
#include "ConfigService.hpp"
#include "ParameterService.h"

using json = nlohmann::json;

FontService *FontService::service = 0;
ModulationService *ModulationService::service = 0;
OscillationService *OscillationService::service = 0;
MidiService *MidiService::service = 0;
ParameterService *ParameterService::service = 0;
ConfigService *ConfigService::service = 0;

//========================================================================
int main( ){
  ofGLFWWindowSettings settings;
  settings.setSize(1400, 900);
  settings.setGLVersion(2, 1);
//  ofSetupOpenGL(1280,720, OF_WINDOW);
//  settings.windowMode = OF_WINDOW;
  auto window = ofCreateWindow(settings);
  auto app = shared_ptr<MainApp>(new MainApp(window));
  ofRunApp(window, app);
  ofRunMainLoop();
}
