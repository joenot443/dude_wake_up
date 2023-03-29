//
//  TextSource.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 3/17/23.
//

#include "TextSource.hpp"
#include "CommonViews.hpp"

void TextSource::setup() {
  fbo.allocate(settings.width->value, settings.height->value, GL_RGBA);
  font.load(displayText->font.path, displayText->fontSize);
  fontPath = displayText->font.path;
}

void TextSource::saveFrame() {
  // Changed resolution
  if (static_cast<int>(fbo.getWidth()) != settings.width->intValue) {
    setup();
  }

  fbo.begin();
  
  // Changed font size
  if (font.getSize() != displayText->fontSize || fontPath != displayText->font.path) {
    fontPath = displayText->font.path;
    font.load(displayText->font.path, displayText->fontSize);
    ofClear(0, 255.);
  }
  
  // Clear if we've moved
  if (xPos != settings.width->intValue * displayText->xPosition->value ||
      yPos != settings.height->intValue * (displayText->yPosition->value)) {
    xPos = settings.width->intValue * displayText->xPosition->value;
    yPos = settings.height->intValue * displayText->yPosition->value;
    ofClear(0, 255.);
  } else {
    ofClear(0.0, 0.0);
  }

  // Draw the text
  ofSetColor(displayText->color);
  font.drawString(displayText->text, xPos, settings.height->intValue - yPos);

  fbo.end();
  
  frameTexture = std::make_shared<ofTexture>(fbo.getTexture());
}

json TextSource::serialize() {
  json j;
  // Serialize TextSource properties
  return j;
}

void TextSource::load(json j) {
  // Load TextSource properties from JSON
}

void TextSource::drawSettings() {
  CommonViews::H3Title("Text Editor");
  ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(5, 5));
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(5., 5.));
  textEditorView.draw();
  ImGui::PopStyleVar();
  ImGui::PopStyleVar();
}
