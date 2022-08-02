//
//  FontService.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 2022-05-10.
//

#include "FontService.hpp"

void FontService::loadFonts() {
  char tmp[256];
  getcwd(tmp, 256);
  std::cout << "Current workin g directory: " << tmp << std::endl;

  ImGuiIO& io = ImGui::GetIO();
  h1 = io.Fonts->AddFontFromFileTTF("Roboto-Medium.ttf", 36);
  h2 = io.Fonts->AddFontFromFileTTF("Roboto-Medium.ttf", 24);
  h3 = io.Fonts->AddFontFromFileTTF("Roboto-Medium.ttf", 18);
  h4 = io.Fonts->AddFontFromFileTTF("Roboto-Bold.ttf", 16);
  p = io.Fonts->AddFontFromFileTTF("Roboto-Regular.ttf", 14);
  b = io.Fonts->AddFontFromFileTTF("Roboto-Bold.ttf", 14);
  i = io.Fonts->AddFontFromFileTTF("Roboto-Italic.ttf", 14);
  int width, height;
  unsigned char* pixels = NULL;
  io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
//  io.Fonts->Build();
}

void FontService::addFontToGui(ofxImGui::Gui *gui) {
  ImGuiIO& io = ImGui::GetIO();
  h1 = gui->addFont("fonts/Roboto-Medium.ttf", 36);
  h2 = gui->addFont("fonts/Roboto-Medium.ttf", 24);
  h3 = gui->addFont("fonts/Roboto-Medium.ttf", 18);
  h4 = gui->addFont("fonts/Roboto-Bold.ttf", 16);
  p = gui->addFont("fonts/Roboto-Regular.ttf", 14);
  b = gui->addFont("fonts/Roboto-Bold.ttf", 14);
  i = gui->addFont("fonts/Roboto-Italic.ttf", 14);
  int width, height;
  unsigned char* pixels = NULL;
  io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
}
