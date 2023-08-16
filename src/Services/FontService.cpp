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

  h1 = gui->addFont("fonts/Roboto-Medium.ttf", 36);
  h2 = gui->addFont("fonts/Roboto-Medium.ttf", 24);
  h3 = gui->addFont("fonts/Roboto-Medium.ttf", 18);
  h4 = gui->addFont("fonts/Roboto-Bold.ttf", 16);
  sm = gui->addFont("fonts/Roboto-Regular.ttf", 12);
  b = gui->addFont("fonts/Roboto-Bold.ttf", 14);
  i = gui->addFont("fonts/Roboto-Italic.ttf", 14);
  p = gui->addFont("fonts/Roboto-Regular.ttf", 14);
  audio = gui->addFont("fonts/fontaudio.ttf", 16, &config, audio_icon_range);
  icon = gui->addFont("fonts/MaterialIcons-Regular.ttf", 16, &config, icon_ranges);
  
  int width, height;
  unsigned char* pixels = NULL;
  io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
}
