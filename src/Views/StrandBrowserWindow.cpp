//
//  StrandBrowserWindow.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 6/30/24.
//

#include "StrandBrowserWindow.hpp"
#include "StrandTileView.hpp"

void StrandBrowserWindow::setup() {
  switch (type) {
    case StrandBrowserType_Recent: {
      strands = StrandService::getService()->availableStrands();
      break;
    }
    case StrandBrowserType_Template: {
      strands = StrandService::getService()->availableTemplateStrands();
      break;
    }
  }
}

void StrandBrowserWindow::update() {
  
}

void StrandBrowserWindow::draw() {
  for (auto strand : strands) {
    StrandTileView::draw(strand);
    ImGui::SameLine();
  }
  ImGui::NewLine();
}
