//
//  MarkdownView.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 2/22/23.
//

#include "MarkdownView.hpp"
#include "ofxImGui.h"
#include "imgui_markdown.h"
#include "FontService.hpp"

static ImGui::MarkdownConfig mdConfig;

void MarkdownView::setup() {
  item = MarkdownService::getService()->itemNamed(name);
  mdConfig.headingFormats[0] = {FontService::getService()->current->h1, true};
  mdConfig.headingFormats[1] = {FontService::getService()->current->h2, true};
  mdConfig.headingFormats[2] = {FontService::getService()->current->h3, true};
}

void MarkdownView::update() {
  
}

void MarkdownView::draw(bool drawCloseButton) {
  ImGui::Markdown(item->contents.c_str(), item->contents.length(), mdConfig);
  if (drawCloseButton && ImGui::Button("Ok")) {
    ImGui::CloseCurrentPopup();
  }
}
