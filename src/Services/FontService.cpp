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
  ImFontConfig config;
  config.MergeMode = false;
  static const ImWchar icon_ranges[] = { 0xe000, 0xF8FF, 0 };
  

  h1 = gui->addFont("../../fonts/Roboto-Medium.ttf", 36);
  
  h2 = gui->addFont("../../fonts/Roboto-Medium.ttf", 24);
  
  h3 = gui->addFont("../../fonts/Roboto-Medium.ttf", 18);
  
  h4 = gui->addFont("../../fonts/Roboto-Bold.ttf", 16);
  
  p = gui->addFont("../../fonts/Roboto-Regular.ttf", 14);
  icon = gui->addFont("../../fonts/OpenFontIcons.ttf", 14, &config, icon_ranges);

  b = gui->addFont("../../fonts/Roboto-Bold.ttf", 14);
  
  i = gui->addFont("../../fonts/Roboto-Italic.ttf", 14);
  
  int width, height;
  unsigned char* pixels = NULL;
  io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
}
