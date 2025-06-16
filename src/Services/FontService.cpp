//
//  FontService.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 2022-05-10.
//

#include "Console.hpp"
#import "ofMain.h"
#include "FontService.hpp"
#include "ConfigService.hpp"
#include "Fonts.hpp"

static const ImWchar icon_ranges[] = { ICON_MIN_MD, ICON_MAX_16_MD, 0 };
static const ImWchar audio_icon_range[] = {ICON_MIN_FAD, ICON_MAX_FAD, 0};


void FontService::setup() {
  loadFonts();
  loadFontSet(retinaFonts, 2.0f);
  loadFontSet(standardFonts, 1.0f);
}

void FontService::useFontSetForScale(float scale) {
  FontSet* target = (scale >= 2.0f) ? &retinaFonts : &standardFonts;
  if (current == target) return;
  
  current = target;

  ImGuiIO& io = ImGui::GetIO();
  io.Fonts = current->atlas;
  io.FontDefault = current->p; // critical!
  io.Fonts->TexID = (ImTextureID)(uintptr_t)current->texID;
  
  // Set the matching style for this DPI
  ImGui::GetStyle() = current->style;
  current->style.ScaleAllSizes(scale);
}

void FontService::loadFontSet(FontSet& set, float scale) {
  set.atlas = new ImFontAtlas();
  
  ImFontConfig cfg;
  cfg.RasterizerDensity = (scale > 1.0f) ? 2.0f : 1.0f;
  cfg.OversampleH = 1;
  cfg.OversampleV = 1;
  cfg.FontDataOwnedByAtlas = true;

  ImFontConfig mergeConfig;
  mergeConfig.MergeMode = false;
  mergeConfig.OversampleH = 1;
  mergeConfig.OversampleV = 1;
  mergeConfig.RasterizerDensity = (scale > 1.0f) ? 2.0f : 1.0f;
  mergeConfig.FontDataOwnedByAtlas = true;
  
  set.h1 = set.atlas->AddFontFromFileTTF(ofToDataPathFS("fonts/Roboto-Medium.ttf").string().c_str(), 72, &cfg);
  set.h2 = set.atlas->AddFontFromFileTTF(ofToDataPathFS("fonts/Roboto-Medium.ttf").string().c_str(), 24, &cfg);
  set.h3 = set.atlas->AddFontFromFileTTF(ofToDataPathFS("fonts/Roboto-Medium.ttf").string().c_str(), 18, &cfg);
  set.h3b = set.atlas->AddFontFromFileTTF(ofToDataPathFS("fonts/Roboto-Bold.ttf").string().c_str(), 18, &cfg);
  set.h4 = set.atlas->AddFontFromFileTTF(ofToDataPathFS("fonts/Roboto-Bold.ttf").string().c_str(), 18, &cfg);
  set.sm = set.atlas->AddFontFromFileTTF(ofToDataPathFS("fonts/Roboto-Regular.ttf").string().c_str(), 12, &cfg);
  set.b = set.atlas->AddFontFromFileTTF(ofToDataPathFS("fonts/Roboto-Bold.ttf").string().c_str(), 16, &cfg);
  set.p = set.atlas->AddFontFromFileTTF(ofToDataPathFS("fonts/Roboto-Regular.ttf").string().c_str(), 16, &cfg);
  set.pN = set.atlas->AddFontFromFileTTF(ofToDataPathFS("fonts/Roboto-Regular.ttf").string().c_str(), 32, &cfg);
  
  set.audio = set.atlas->AddFontFromFileTTF(ofToDataPathFS("fonts/fontaudio.ttf").string().c_str(), 16, &cfg, audio_icon_range);
  set.icon = set.atlas->AddFontFromFileTTF(ofToDataPathFS("fonts/MaterialIcons-Regular.ttf").string().c_str(), 16, &mergeConfig, icon_ranges);
  set.largeIcon = set.atlas->AddFontFromFileTTF(ofToDataPathFS("fonts/MaterialIcons-Regular.ttf").string().c_str(), 24, &mergeConfig, icon_ranges);
  set.xLargeIcon = set.atlas->AddFontFromFileTTF(ofToDataPathFS("fonts/MaterialIcons-Regular.ttf").string().c_str(), 72, &mergeConfig, icon_ranges);
  
  unsigned char* pixels;
  int width, height;
  set.atlas->GetTexDataAsRGBA32(&pixels, &width, &height);
  
  glGenTextures(1, &set.texID);
  glBindTexture(GL_TEXTURE_2D, set.texID);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glBindTexture(GL_TEXTURE_2D, 0);
  
  set.atlas->TexID = (ImTextureID)(uintptr_t)set.texID;
  
  float dpiScale = scale; // 1.0 or 2.0 etc
  ImGuiStyle base = ImGui::GetStyle();
  set.style = base;
  set.style.ScaleAllSizes(dpiScale);
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
