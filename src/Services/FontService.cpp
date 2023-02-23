//
//  FontService.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 2022-05-10.
//

#include "FontService.hpp"
#include "Fonts.hpp"

void FontService::addFontToGui(ofxImGui::Gui *gui) {
  ImGuiIO& io = ImGui::GetIO();
  io.Fonts->AddFontDefault();
  
  ImFontConfig config;
  config.MergeMode = false;
//  config.PixelSnapH = true;
  
  static const ImWchar icon_ranges[] = { ICON_MIN_MD, ICON_MAX_16_MD, 0 };
  static const ImWchar audio_icon_range[] = {ICON_MIN_FAD, ICON_MAX_FAD, 0};

  h1 = gui->addFont("Roboto-Medium.ttf", 36);
  
  h2 = gui->addFont("Roboto-Medium.ttf", 24);
  
  h3 = gui->addFont("Roboto-Medium.ttf", 18);
  
  h4 = gui->addFont("Roboto-Bold.ttf", 16);

  sm = gui->addFont("Roboto-Regular.ttf", 10);

  b = gui->addFont("Roboto-Bold.ttf", 14);

  i = gui->addFont("Roboto-Italic.ttf", 14);
  
  p = gui->addFont("Roboto-Regular.ttf", 14);
  
  audio = gui->addFont("fontaudio.ttf", 16, &config, audio_icon_range);
  icon = gui->addFont("MaterialIcons-Regular.ttf", 16, &config, icon_ranges);
  
  int width, height;
  unsigned char* pixels = NULL;
  io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
}
