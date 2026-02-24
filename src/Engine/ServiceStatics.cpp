//
//  ServiceStatics.cpp
//  dude_wake_up
//
//  Static member definitions for all singleton services.
//  These were originally in main.cpp but are extracted here so they can
//  be compiled by both the dude_wake_up and NottawaApp targets.
//

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
#include "TimeService.hpp"
#include "ParameterService.hpp"
#include "IconService.hpp"
#include "MainApp.h"

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
TimeService *TimeService::service = 0;
MainApp *MainApp::app = 0;
