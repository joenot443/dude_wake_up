//
//  FontService.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 2022-05-10.
//

#include "Console.hpp"
#import "ofMain.h"
#include "FontService.hpp"
#include "Fonts.hpp"

void FontService::addFontToGui(ofxImGui::Gui *gui) {
  ImGuiIO& io = ImGui::GetIO();
  io.Fonts->AddFontDefault();
  
  ImFontConfig mergeConfig;
  mergeConfig.MergeMode = false;
  
  float scale = static_cast<float>(dynamic_cast<ofAppGLFWWindow*>(ofGetWindowPtr())->getPixelScreenCoordScale());
  ImFontConfig rasterConfig;
  log("%.2f", scale);
  
  // Account for Retina displays
  rasterConfig.RasterizerDensity = scale > 1.0f ? 4.0f : 1.0f;
  
  static const ImWchar icon_ranges[] = { ICON_MIN_MD, ICON_MAX_16_MD, 0 };
  static const ImWchar audio_icon_range[] = {ICON_MIN_FAD, ICON_MAX_FAD, 0};

  h1 = gui->addFont("fonts/Roboto-Medium.ttf", 72, &rasterConfig);
  h2 = gui->addFont("fonts/Roboto-Medium.ttf", 24, &rasterConfig);
  h3 = gui->addFont("fonts/Roboto-Medium.ttf", 18, &rasterConfig);
  h3b = gui->addFont("fonts/Roboto-Bold.ttf", 18, &rasterConfig);
  h4 = gui->addFont("fonts/Roboto-Bold.ttf", 18, &rasterConfig);
  sm = gui->addFont("fonts/Roboto-Regular.ttf", 12, &rasterConfig);
  b = gui->addFont("fonts/Roboto-Bold.ttf", 16, &rasterConfig);
  i = gui->addFont("fonts/Roboto-Italic.ttf", 16, &rasterConfig);
  p = gui->addFont("fonts/Roboto-Regular.ttf", 16, &rasterConfig);
  pN = gui->addFont("fonts/Roboto-Regular.ttf", 32, &rasterConfig);
  audio = gui->addFont("fonts/fontaudio.ttf", 16, &mergeConfig, audio_icon_range);
  icon = gui->addFont("fonts/MaterialIcons-Regular.ttf", 16, &mergeConfig, icon_ranges);
  largeIcon = gui->addFont("fonts/MaterialIcons-Regular.ttf", 24, &mergeConfig, icon_ranges);
  xLargeIcon = gui->addFont("fonts/MaterialIcons-Regular.ttf", 72, &mergeConfig, icon_ranges);
  
  int width, height;
  unsigned char* pixels = NULL;
  io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
}

void FontService::loadFonts() {
  ofDirectory fontsDir = ofDirectory("fonts/editor");
  fontsDir.listDir();
  // Collect font names
  for (auto& file : fontsDir.getFiles()) {
    fonts.push_back(Font(removeFileExtension(file.getFileName()), file.getAbsolutePath()));
  }
  // Sort by name
  std::sort(fonts.begin(), fonts.end(), [](const Font& a, const Font& b) {
    return a.name < b.name;
  });
  
  for (auto font : fonts) {
   fontNames.push_back(font.name);
  }
}
